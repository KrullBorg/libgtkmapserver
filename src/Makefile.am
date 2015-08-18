LIBS = $(GTKMAPSERVER_LIBS)

AM_CPPFLAGS = $(GTKMAPSERVER_CFLAGS) \
              -DLOCALEDIR=\"$(localedir)\" \
              -DG_LOG_DOMAIN=\"GtkMapserver\"

lib_LTLIBRARIES = libgtkmapserver.la

libgtkmapserver_la_SOURCES = gtkmapserver.c

libgtkmapserver_la_LDFLAGS = -no-undefined

libgtkmapserver_include_HEADERS = gtkmapserver.h

libgtkmapserver_includedir = $(includedir)/libgtkmapserver
