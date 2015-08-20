if PLATFORM_WIN32
iconsdir = `cygpath \`pkg-config --variable=pixmapdir gladeui-2.0\``/hicolor/22x22/actions
else
iconsdir = `pkg-config --variable=pixmapdir gladeui-2.0`/hicolor/22x22/actions
endif

icons_DATA = widget-gtkmapserver-gtkmapserver.png

EXTRA_DIST = $(icons_DATA)
