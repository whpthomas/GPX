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

// s3g.h
// Structure and routine definitions for the s3g library

#ifndef S3G_H_

#define S3G_H_

#include <inttypes.h>
#include "s3g_commands.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef S3G_CONTEXT_T_
#define S3G_CONTEXT_T_
typedef void s3g_context_t;
#endif

// s3g_queue_point_abs, s3g_queue_point_ext, and s3g_queue_point_new
// all have the same size, packing and layout.  They can be cast to
// one another and the relevant fields extracted.  The dummy_ fields
// will be set to zero when appropriate.
typedef struct {
     int32_t  x;
     int32_t  y;
     int32_t  z;
     int32_t  dummy_a;
     int32_t  dummy_b;
     int32_t  dda;
     uint8_t  dummy_rel;
     float    dummy_distance;
     uint16_t dummy_feedrate_mult_64;
} s3g_queue_point_abs;

typedef struct {
     int32_t  x;
     int32_t  y;
     int32_t  z;
     int32_t  a;
     int32_t  b;
     int32_t  dda;
     uint8_t  dummy_rel;
     float    dummy_distance;
     uint16_t dummy_feedrate_mult_64;
} s3g_queue_point_ext;

typedef struct {
     int32_t  x;
     int32_t  y;
     int32_t  z;
     int32_t  a;
     int32_t  b;
     int32_t  dda_rate;
     uint8_t  rel;
     float    distance;
     uint16_t feedrate_mult_64;
} s3g_queue_point_new_ext;

typedef struct {
     int32_t  x;
     int32_t  y;
     int32_t  z;
     int32_t  a;
     int32_t  b;
     int32_t  us;
     uint8_t  rel;
     float    dummy_distance;
     uint16_t dummy_feedrate_mult_64;
} s3g_queue_point_new;

typedef struct {
     uint8_t index;
} s3g_change_tool;

typedef struct {
     uint8_t axes;
} s3g_enable_axes;

typedef struct {
     int32_t x;
     int32_t y;
     int32_t z;
} s3g_set_position;

typedef struct {
     int32_t x;
     int32_t y;
     int32_t z;
     int32_t a;
     int32_t b;
} s3g_set_position_ext;

typedef struct {
     uint32_t millis;
} s3g_delay;

typedef struct {
     uint8_t  flags;
     uint32_t feedrate; // microseconds per step
     uint16_t timeout;   // seconds
} s3g_find_axes_minmax;

typedef struct {
     uint8_t  index;
     uint16_t ping_delay;
     uint16_t timeout;
} s3g_wait_for_tool;

typedef struct {
     uint8_t  index;
     uint16_t ping_delay;
     uint16_t timeout;
} s3g_wait_for_platform;

typedef struct {
     uint8_t axes;
} s3g_store_home_position;

typedef struct {
     uint8_t axes;
} s3g_recall_home_position;

typedef struct {
     uint8_t     index;
     uint8_t     subcmd_id;
     size_t      subcmd_len;
     const char *subcmd_desc;
     uint16_t    subcmd_value;
} s3g_tool;

typedef struct {
     int32_t x;
     int32_t y;
     int32_t z;
     int32_t a;
} s3g_set_max_accel;

typedef struct {
     int32_t x;
     int32_t y;
     int32_t z;
     int32_t a;
} s3g_set_max_feedrate;

typedef struct {
     int32_t s;
     int32_t t;
} s3g_set_default_accel;

typedef struct {
     int32_t s;
     int32_t t;
     int32_t x;
     int32_t z;
} s3g_set_advanced_accel;

typedef struct {
     int32_t d;
} s3g_set_filament_diameter;

typedef struct {
     int32_t s;
     int32_t k;
} s3g_set_advance_k;

typedef struct {
     int32_t a;
} s3g_set_extruder_stepsmm;

typedef struct {
     uint8_t s;
} s3g_set_segment_acceleration;

typedef struct {
     int32_t r;
     int32_t g;
     int32_t b;
     int32_t fade_speed;
     int32_t write_to_eeprom;
} s3g_mood_set_rgb;

typedef struct {
     int32_t h;
     int32_t s;
     int32_t b;
     int32_t write_to_eeprom;
} s3g_mood_set_hsb;

typedef struct {
     int32_t script_id;
     int32_t write_to_eeprom;
} s3g_mood_play_script;

typedef struct {
     uint8_t repeats;
} s3g_buzzer_repeats;

typedef struct {
     uint8_t buzzes;
     uint8_t duration;
     uint8_t repeats;
} s3g_buzzer_buzz;

typedef struct {
     int64_t x; // int32_t * 10000
     int64_t y; // int32_t * 10000
     int64_t z; // int32_t * 10000
     int64_t a; // int32_t * 10000
} s3g_set_axis_steps_mm;

typedef struct {
     uint8_t axis;
     uint8_t value;
} s3g_set_pot_value;

typedef struct {
     uint8_t red;
     uint8_t green;
     uint8_t blue;
     uint8_t blink_rate;
     uint8_t effect;
} s3g_set_rgb_led;

typedef struct {
     uint16_t frequency;
     uint16_t duration;
     uint8_t effect;
} s3g_set_beep;

typedef struct {
     uint8_t  mask;
     uint16_t timeout;
     uint8_t  timeout_behavior;
} s3g_pause_for_button;

typedef struct {
     uint8_t       options;
     uint8_t       x;
     uint8_t       y;
     uint8_t       timeout;
     uint8_t       message_len;
     unsigned char message[64];
} s3g_display_message;

typedef struct {
     uint8_t percentage;
     uint8_t reserved;
} s3g_set_build_percent;

typedef struct {
     uint8_t song_id;
} s3g_queue_song;

typedef struct {
     uint8_t options;
} s3g_reset_to_factory;

typedef struct {
     uint32_t      steps;
     uint8_t       message_len;
     unsigned char message[64];
} s3g_build_start_notification;

typedef struct {
     uint8_t flags;
} s3g_build_end_notification;

typedef struct {
    uint8_t  version_high;
    uint8_t  version_low;
    uint8_t  reserved1;
    uint32_t reserved2;
    uint16_t bot_type;
    uint16_t reserved3;
    uint32_t reserved4;
    uint32_t reserved5;
    uint8_t  reserved6;
} s3g_stream_version;

typedef struct {
    float zpos;
} s3g_pause_at_zpos;

// s3g_command_t
// An individual command read from a .s3g file is stored in
// this data structure.  You need to know from the command id
// which member of the union to look at.

#define MAX_S3G_CMD_LEN 32

// #define constants from the command ids are available in Commands.hh

typedef struct {
     uint8_t        cmd_id;
     size_t         cmd_len;
     const char    *cmd_desc;
     size_t         cmd_raw_len;
     unsigned char  cmd_raw[MAX_S3G_CMD_LEN];
     union {
	  s3g_queue_point_abs          queue_point_abs;
	  s3g_queue_point_ext          queue_point_ext;
	  s3g_queue_point_new          queue_point_new;
	  s3g_queue_point_new_ext      queue_point_new_ext;
	  s3g_change_tool              change_tool;
	  s3g_enable_axes              enable_axes;
	  s3g_set_position             set_position;
	  s3g_set_position_ext         set_position_ext;
	  s3g_delay                    delay;
	  s3g_find_axes_minmax         find_axes_minmax;
	  s3g_wait_for_tool            wait_for_tool;
	  s3g_wait_for_platform        wait_for_platform;
	  s3g_store_home_position      store_home_position;
	  s3g_recall_home_position     recall_home_position;
	  s3g_tool                     tool;
	  s3g_set_max_accel            set_max_accel;
	  s3g_set_max_feedrate         set_max_feedrate;
	  s3g_set_default_accel        set_default_accel;
	  s3g_set_advanced_accel       set_advanced_accel;
	  s3g_set_filament_diameter    set_filament_diameter;
	  s3g_set_advance_k            set_advance_k;
	  s3g_set_extruder_stepsmm     set_extruder_stepsmm;
	  s3g_set_segment_acceleration set_segment_acceleration;
	  s3g_mood_set_rgb             mood_set_rgb;
	  s3g_mood_set_hsb             mood_set_hsb;
	  s3g_mood_play_script         mood_play_script;
	  s3g_buzzer_repeats           buzzer_repeats;
	  s3g_buzzer_buzz              buzzer_buzz;
	  s3g_set_axis_steps_mm        set_axis_steps_mm;
	  s3g_set_pot_value            digi_pot;
	  s3g_set_rgb_led              rgb_led;
	  s3g_set_beep                 beep;
	  s3g_pause_for_button         button_pause;
	  s3g_display_message          display_message;
	  s3g_set_build_percent        build_percentage;
	  s3g_queue_song               queue_song;
	  s3g_reset_to_factory         factory_reset;
	  s3g_build_start_notification build_start;
	  s3g_build_end_notification   build_end;
	  s3g_stream_version           x3g_version;
	  s3g_pause_at_zpos            pause_at_zpos;
     } t;
} s3g_command_t;

#define S3G_INPUT_TYPE_FILE 0  // stdin or a named disk file

// Obtain an s3g_context for an input source of type S3G_INPUT_TYPE_.
// The context returned must be disposed of by calling s3g_close().
//
// Call arguments:
//
//   int type
//      Input source type.  Must be one of
//
//         S3G_INPUT_TYPE_FILE
//
//   void *src
//      Input source information for the selected input type
//
//         S3G_INPUT_TYPE_FILE -- const char *filename or NULL for stdin
//
//   Return values:
//
//     != NULL -- Success
//     == NULL -- Error; consult errno

s3g_context_t *s3g_open(int type, const char *src, int create_file, int mode);


// Read a single command from the s3g context, storing the result in the
// supplied s3g_command_t structure
//
// Call arguments:
//
//   s3g_context_t *ctx
//     Context obtained by calling s3g_open().
//
//   s3g_command_t *cmd
//     Pointer to a s3g_command_t structure to store the read command in.  If NULL
//     is passed for this argument, then the command is read but no information about
//     the command is returned.
//
//  Return values:
//
//    0 -- Success
//    1 -- Unrecognized command encountered; not possible to read further
//   -1 -- Read error; check errno

int s3g_command_read(s3g_context_t *ctx, s3g_command_t *cmd);

int s3g_command_read_ext(s3g_context_t *ctx, s3g_command_t *cmd,
			 unsigned char *rawbuf, size_t maxbuf, size_t *len);


// Close the s3g input source, releasing any resources
//
// Call arguments:
//
//   s3g_context_t *ctx
//     Context obtained by calling s3g_open().
//
//  Return values:
//
//    0 -- Success
//   -1 -- Close error; check errno

int s3g_close(s3g_context_t *ctx);

#ifndef S3G_PRIVATE_H_
typedef ssize_t s3g_write_proc_t(void *ctx, void *buf, size_t nbytes);
#endif

int s3g_add_writer(s3g_context_t *ctx, s3g_write_proc_t *wproc, void *wctx);
void s3g_command_display(s3g_context_t *ctx, s3g_command_t *cmd);

int s3g_command_write(s3g_context_t *ctx, s3g_command_t *cmd);

int s3g_command_isblocking(s3g_command_t *cmd);

#ifdef __cplusplus
}
#endif

#endif
