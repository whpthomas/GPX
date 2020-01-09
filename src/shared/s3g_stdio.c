/*  Copyright (c) 2015, Dan Newman <dan(dot)newman(at)mtbaldy(dot)us>
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer. 
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "s3g_stdio.h"

// Identify temporary read errors
// Can be os-specific

#if !defined(_WIN32) && !defined(_WIN64)

#if defined(ENOSR)
#define ERRNO_EQ_ENOSR errno == ENOSR
#else
#define ERRNO_EQ_ENOSR 0
#endif

#define FD_TEMPORARY_ERR() \
     (errno == EINTR || errno == ENOMEM || errno == ENOBUFS || ERRNO_EQ_ENOSR)
#define FD_WOULDBLOCK_ERR() \
     (errno == EWOULDBLOCK || errno == EAGAIN)

#else

#define FD_TEMPORARY_ERR() \
     (errno == EINTR || errno == ENOMEM )
#define FD_WOULDBLOCK_ERR() \
     (errno == EAGAIN)

#endif

// This driver's private context

typedef struct {
     int    fd;  // File descriptor; < 0 indicates that the file is not open
     size_t nread;
     size_t nwritten;
} s3g_rw_stdio_ctx_t;


// stdio_close
//
// Close the input source and release the allocated driver context
//
// Call arguments:
//
//   void *ctx
//     Private driver context allocated by stdio_open().
//
// Return values:
//
//   0 -- Success
//  -1 -- Error; check errno may be a close() error which is significant
//          when writing

static s3g_close_proc_t stdio_close;
static int stdio_close(void *ctx)
{
     int fd;
     s3g_rw_stdio_ctx_t *myctx = (s3g_rw_stdio_ctx_t *)ctx;

     // Sanity check
     if (!myctx)
     {
	  errno = EINVAL;
	  return(-1);
     }

     fd = myctx->fd;
     free(myctx);

     if (fd < 0)
     {
	  errno = EBADF;
	  return(-1);
     }

     return(close(fd));
}


// stdio_read_retry
//
// Read the specified number of bytes into the buffer.  Temporary read errors
// wil be handled by this routine which will try to read all nbytes unless EOF
// is encountered.
//
// When the end of the file is reached before reading nbytes, the return value,
// R, will satisfy 0 <= R < nbytes.
//  
// *** Length checking is not performed on the buffer
// *** That is presumed to be done by stdio_read()
//
// Call arguments:
//
//   int fd
//     Descriptor of the open file to read from.
//
//   unsigned char *buf
//     Pointer to a buffer to read the data to.  Length of the buffer must be
//     at least nbytes long.  Buffer will NOT be NUL terminated.
//
//   size_t nbytes
//     The number of bytes to read from the input source.
//
// Return values:
//
//   > 0 -- Number of bytes read.  Guaranteed to be == nbytes UNLESS a
//            permanent read error occurs or the end of the file is reached.
//     0 -- End of file reached (unless nbytes == 0)
//    -1 -- File error; check errno

static ssize_t stdio_read_retry(int fd, void *buf, size_t nbytes)
{
     ssize_t n, nread;

     // Save read() the bother of this test
     if (fd < 0)
     {
	  errno = EBADF;
	  return((ssize_t)-1);
     }

     // Repeatedly call until we've read the requested amount of data or gotten
     // an error which isn't temporary
     nread = 0;
     for (;;)
     {
	  if ((n = read(fd, buf, nbytes)) <= 0 && FD_TEMPORARY_ERR())
	       continue;
	  if (n == 0)
	       // EOF reached
	       return(nread);
	  nread  += n;
	  nbytes -= n;
	  if (nbytes == 0)
	       return(nread);
	  buf += n;
     }
}


// stdio_read
//
// Read the specified number of bytes from the input source, placing at most
// maxbuf bytes into the buffer, buf.  If nbytes > maxbuf, then nbytes will be
// read.  However, only the first maxbuf bytes read will be stored in buf.
// Note that buf will not be NUL terminated.
//
// Unless a permanent error occurs, stdio_read() guarantees that nbytes will
// actually be read.  If less than nbytes is returned, then either an
// end-of-file condition occurred or there was a non-temporary read error.
//
// On return, stdio_read() indicates how many bytes were actually read.
//
// Call arguments:
//
//   void *ctx
//     Private driver context created by stdio_open().
//
//   void *buf
//     Buffer into which to read the data.  At most maxbuf bytes will be
//     stored in this buffer.  Any bytes read beyond maxbuf will be discarded.
//     That only occurs when nbytes > maxbuf.  If buf == NULL, then maxbuf will
//     be considered 0 and nbytes will be read and discarded.
//
//   size_t maxbuf
//     Maximum number of bytes to store in buf.  Maxbuf must not exceed the
//     size of the buffer pointed at by buf.
//
//   size_t nbytes
//     The number of bytes to read from the input source.
//
// Return values:
//
//  > 0 -- Number of bytes read.  If the returned value is less than nbytes,
//           then an end of file condition has occurred.
//    0 -- End of file reached or nbytes == 0
//   -1 -- Read error or invalid call arguments; check errno
//
static s3g_read_proc_t stdio_read;
static ssize_t stdio_read(void *ctx, void *buf, size_t maxbuf, size_t nbytes)
{
     s3g_rw_stdio_ctx_t *myctx = (s3g_rw_stdio_ctx_t *)ctx;
     ssize_t n;

     // Sanity check
     if (!myctx)
     {
	  errno = EINVAL;
	  return((ssize_t)-1);
     }
     else if (myctx->fd < 0)
     {
	  errno = EBADF;
	  return((size_t)-1);
     }

     // Return now if nothing to read
     if (nbytes == 0)
	  return((ssize_t)0);

     // Treat NULL for buf as though maxbuf == 0
     if (!buf)
	  maxbuf = 0;

     // Buffer is big enough to contain the entire read
     if (nbytes <= maxbuf)
	  return(stdio_read_retry(myctx->fd, buf, nbytes));

     // Buffer is not large enough to contain the entire read
     if ((n = stdio_read_retry(myctx->fd, buf, maxbuf)) <= 0)
	  return(n);

     // Read the remaining number of bytes requested without
     // shoving them into buf (which is full)
     nbytes -= n;
     {
	  unsigned char tmpbuf[1024];
	  size_t nread;

	  while (nbytes != 0)
	  {
	       
	       nread = (sizeof(tmpbuf) < nbytes) ?
		    (size_t)sizeof(tmpbuf) : nbytes;
	       if ((n = stdio_read_retry(myctx->fd, tmpbuf, nread)) <= 0)
		    return(n);
	       // NOTE: stdio_read_retry() guarantees n == nread when n > 0
	       nbytes       -= n;
	       myctx->nread += n;
	  }
     }
     return(0);
}



// stdio_write
//
// Write the first nbytes from the buffer buf to the underlying file descriptor.
//
// Unless a permanent error occurs, stdio_write() guarantees that nbytes will
// actually be written.  If less than nbytes are written, then a permanent
// error of some flavor has occurred.
//
// On return, stdio_write() indicates how many bytes were actually read or
// -1 in the event of a permanent error.  Check errno for details on any
// returned error.
//
// Call arguments:
//
//   void *ctx
//     Private driver context created by stdio_open().
//
//   const void *buf
//     Buffer of data to write.  At most nbytes bytes will be written from
//     this buffer.  Used for input only.
//
//   size_t nbytes
//     The number of bytes to write from the buffer, buf.  Used for input
//     only.
//
// Return values:
//
//  > 0 -- Number of bytes written.  If the returned value is less than
//           nbytes, then a permanent error condition has occurred.
//    0 -- Nothing written; nbytes == 0
//   -1 -- Write error or invalid call arguments supplied; check errno
//
static s3g_write_proc_t stdio_write;
static ssize_t stdio_write(void *ctx, const void *buf, size_t nbytes)
{
     s3g_rw_stdio_ctx_t *myctx = (s3g_rw_stdio_ctx_t *)ctx;
     ssize_t nwritten;

     // Sanity check
     if (!myctx || !buf)
     {
	  errno = EINVAL;
	  return((ssize_t)-1);
     }
     else if (myctx->fd < 0)
     {
	  errno = EBADF;
	  return((size_t)-1);
     }

     // Return now if nothing to read
     if (nbytes == 0)
	  return((ssize_t)0);

     // Read the remaining number of bytes requested without
     // shoving them into buf (which is full)
     nwritten = 0;
     while (nbytes)
     {
	  ssize_t nw;

	  nw = write(myctx->fd, buf, nbytes);
	  if (nw <= 0)
	  {
	       if (FD_WOULDBLOCK_ERR())
		    continue;
	       return((ssize_t)-1);
	  }
	  nwritten        += nw;
	  nbytes          -= nw;
	  myctx->nwritten += nw;
	  buf = (const void *)((char *)buf + nw);
     }

     return(nwritten);
}

// s3g_stdio_open
// Our public open routine.  This is the only public routine for the driver.
//
// Call arguments
//
//   s3g_context_t *ctx
//     s3g context to associate ourselves with.
//
//   void *src
//     Input source information.  For this driver, a value of NULL indicates
//     that the input source is stdin (create_file == 0) or stdout
//     (create_file != 0).  Otherwise, the value is treated as a "const char *"
//     pointer pointing to the name of a file to open in read only mode.  A
//     ".s3g" will NOT be appended to the file name.  The file name must be
//     the complete file name (but need not be an absolute file path).
//
//   int create_file
//     If zero, then the file is opened for reading only.  If non-zero, the
//     file is created and opened for writing only.  Used for input only.
//
//   int mode
//     open(2) permission mask, mode, used when creating a new file.  Used for
//     input only.
//
// Return values:
//
//   0 -- Success
//  -1 -- Error; check errno

int s3g_stdio_open(s3g_context_t *ctx, const char *src, int create_file, int mode)
{
     s3g_rw_stdio_ctx_t *tmp;

     // Sanity check
     if (!ctx)
     {
	  fprintf(stderr, "s3g_stdio_open(%d): Invalid call; ctx=NULL\n",
		  __LINE__);
	  errno = EINVAL;
	  return(-1);
     }

     // Allocate memory for our "driver" context
     tmp = (s3g_rw_stdio_ctx_t *)calloc(1, sizeof(s3g_rw_stdio_ctx_t));
     if (tmp == NULL)
     {
	  fprintf(stderr, "s3g_open(%d): Unable to allocate VM; %s (%d)\n",
		  __LINE__, strerror(errno), errno);
	  return(-1);
     }

     // What sort of input source: named file or stdin?
     if (src == NULL)
     {
	  // Assume we're using stdin
	  tmp->fd = create_file ? fileno(stdout) : fileno(stdin);
     }
     else
     {
	  const char *fname = (const char *)src;
	  int oflag =  create_file ? O_CREAT | O_WRONLY : O_RDONLY;
#ifdef O_BINARY
          if (!create_file)
              oflag |= O_BINARY;
#endif
	  int fd = open(fname, oflag, mode);
	  if (fd < 0)
	  {
	       fprintf(stderr, "s3g_open(%d): Unable to open the file \"%s\"; "
		       "%s (%d)\n",
		       __LINE__, fname, strerror(errno), errno);
	       free(tmp);
	       return(-1);
	  }
	  tmp->fd = fd;
     }

     // All finished and happy
     ctx->close  = stdio_close;
     ctx->read   = stdio_read;
     ctx->r_ctx  = tmp;
     ctx->write  = create_file ? stdio_write : NULL;
     ctx->w_ctx  = create_file ? tmp : NULL;

     return(0);
}
