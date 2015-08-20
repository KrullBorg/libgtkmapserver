SUBDIRS = icons


gladecatalogs_DATA = gtkmapserver.xml

EXTRA_DIST = $(gladecatalogs_DATA)

if PLATFORM_WIN32
glademodulesdir = `cygpath \`pkg-config --variable=moduledir gladeui-2.0\``
gladecatalogsdir = `cygpath \`pkg-config --variable=catalogdir gladeui-2.0\``
libmodulesext = -0.dll
libmodulesextglade = .dll
libmodulesdir = $(bindir)
else
glademodulesdir = `pkg-config --variable=moduledir gladeui-2.0`
gladecatalogsdir = `pkg-config --variable=catalogdir gladeui-2.0`
libmodulesext = .so
libmodulesextglade = $(libmodulesext)
libmodulesdir = $(libdir)
endif

install-exec-hook:
	cd $(glademodulesdir) && \
	ln -s -f $(libmodulesdir)/libgtkmapserver$(libmodulesext) libgtkmapserver$(libmodulesextglade)

uninstall-hook:
	rm -f $(glademodulesdir)/libgtkmapserver$(libmodulesext)
