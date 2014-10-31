#
# libetnaviv - etnaviv userspace library
#
# Written by Wladimir J. van der Laan 2013
#

# Turn off -Wnested-externs - these are a good thing because it allows
# information hiding and helps prevent misuse of private externs.
# Turn off -Wcast-qual - this makes stuff like string assignment
# too noisy.
# Turn off -Wredundant-decls - Xorg headers seem to contain a lot
# of this, so why it's in xorg-macros.m4... maybe more of a wish?
# Turn off -Wshadow - Xorg headers seem to declare a lot of globals
# which can conflict - index, range, etc.
AM_CFLAGS = $(filter-out -Wnested-externs -Wcast-qual -Wredundant-decls \
        -Werror=write-strings -Wshadow,$(CWARNFLAGS)) \
        -std=gnu99 -Wall $(GALCORE_CFLAGS) $(VIVHOOK_CFLAGS) \
	-I$(top_srcdir)/src

libetnaviv_la_LTLIBRARIES = libetnaviv.la
libetnaviv_ladir = $(libdir)
libetnaviv_la_CFLAGS = $(AM_CFLAGS)
libetnaviv_la_LDFLAGS = -version-info 1:0:0 -no-undefined 
libetnaviv_la_LIBADD = $(GALCORE_LIBS) $(VIVHOOK_LIBS)

libetnaviv_la_SOURCES = \
			etna.c \
			viv.c \
			viv_profile.c \
			etna_bo.c \
			etna_queue.c \
			etna_tex.c \
			etna_fb.c \
			etna_rs.c

libetnaviv_la_includedir = $(includedir)/etnaviv
libetnaviv_la_include_HEADERS = \
			cmdstream.xml.h \
			common.xml.h \
			etna_bo.h \
			etna_fb.h \
			etna.h \
			etna_queue.h \
			etna_rs.h \
			etna_tex.h \
			etna_util.h \
			interval.h \
			isa.xml.h \
			state_2d.xml.h \
			state_3d.xml.h \
			state_hi.xml.h \
			state_vg.xml.h \
			state.xml.h \
			viv.h \
			viv_internal.h \
			viv_profile.h
