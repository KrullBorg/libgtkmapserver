LIBS = $(GTKMAPSERVER_LIBS) \
       -export-dynamic

AM_CPPFLAGS = $(GTKMAPSERVER_CFLAGS) \
              -I$(top_srcdir)/src \
              -DTESTSDIR="\"@abs_builddir@\""

noinst_PROGRAMS = gtkmapserver

LDADD = $(top_builddir)/src/libgtkmapserver.la

EXTRA_DIST =
