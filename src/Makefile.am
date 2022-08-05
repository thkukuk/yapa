#
# Copyright (c) 2006 Thorsten Kukuk, Germany
#
# Author: Thorsten Kukuk <kukuk@thkukuk.de>
#

localedir = $(datadir)/locale

WARNFLAGS = @WARNFLAGS@
AM_CFLAGS = $(WARNFLAGS) -DLOCALEDIR=\"$(localedir)\"
LDADD = @IMLIB2_LIBS@ @EXIF_LIBS@

CLEANFILES = *~

bin_PROGRAMS = yapa

noinst_HEADERS = main.h

yapa_SOURCES = main.c images.c directories.c txtnotes.c \
	htmlfiles.c config.c exif.c style.c gpx-tracks.c
