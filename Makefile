# Declaration of variables
CC = cc
CFLAGS = -w
LIBS = -lm

# File names
VERSION = 2.0
PLATFORM=osx
ARCHIVE = gpx-$(PLATFORM)-$(VERSION)
PREFIX = /usr/local
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

all: gpx

.PHONY: all

# Main target
gpx: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o gpx

# To obtain object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# To remove generated files
clean:
	rm -f gpx $(OBJECTS)
	rm -f $(ARCHIVE).tar.gz
	rm -f $(ARCHIVE).zip
	rm -f $(ARCHIVE).dmg

# To install program and supporting files
install: gpx
	test -d $(PREFIX) || mkdir $(PREFIX)
	test -d $(PREFIX)/bin || mkdir $(PREFIX)/bin
	install -m 0755 gpx $(PREFIX)/bin
#	test -d $(PREFIX)/share || mkdir $(PREFIX)/share
#	test -d $(PREFIX)/share/gpx || mkdir -p $(PREFIX)/share/gpx
#	for INI in *.ini; do \
#		install -m 0644 $$INI $(PREFIX)/share/gpx; \
#	done

# To make a distribution archive
release: gpx
	rm -rf $(ARCHIVE)	# Get rid of previous junk, if any.
	rm -f $(ARCHIVE).tar.gz
	rm -f $(ARCHIVE).zip
	rm -f $(ARCHIVE).dmg
	mkdir $(ARCHIVE)
	cp -r gpx examples scripts *.ini $(ARCHIVE)
	tar cf - $(ARCHIVE) | gzip -9c > $(ARCHIVE).tar.gz
	zip -r $(ARCHIVE).zip $(ARCHIVE)
	test -f /usr/bin/hdiutil && hdiutil create -format UDZO -srcfolder $(ARCHIVE) $(ARCHIVE).dmg
	rm -rf $(ARCHIVE)


# Run unit test
test: gpx
	./gpx examples/lint.gcode
	python scripts/s3g-decompiler.py examples/lint.x3g
