INCLUDES  = $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)

## The common lib, shared between the part, the plugins, and the filters
lib_LTLIBRARIES = libkritacommon.la
libkritacommon_la_SOURCES = dummy.cc
libkritacommon_la_LDFLAGS = $(all_libraries) -version-info 1:0 -no-undefined
libkritacommon_la_LIBADD = sdk/libkritasdk.la core/libkritaimage.la ui/libkritaui.la kritacolor/libkritacolor.la $(LCMS_LIBS) $(LIB_KOFFICEUI) $(LIB_KOPAINTER) $(LIB_KOPALETTE) $(LIB_XINPUTEXT)

## The part
kde_module_LTLIBRARIES = libkritapart.la
libkritapart_la_SOURCES = krita_part_init.cc
libkritapart_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
libkritapart_la_LIBADD = libkritacommon.la

METASOURCES = AUTO

## The kdeinit loadable module and executable
kdeinit_LTLIBRARIES = krita.la
bin_PROGRAMS =
krita_la_SOURCES = main.cc
krita_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
krita_la_LIBADD = $(LIB_KOFFICECORE)

xdg_apps_DATA = krita.desktop
kdemimedir = $(kde_mimedir)/application

rcdir = $(kde_datadir)/krita
rc_DATA = krita.rc krita_readonly.rc

SUBDIRS = sdk kritacolor core ui . dtd colorspaces plugins data pics
# Needed to compile libkritacommon.la, which has no source files to itself
# but everything in static libs.
dummy.cc:
	echo > dummy.cc

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui` >> rc.cpp
	$(EXTRACTRC) `find . -name \*.rc` >> rc.cpp
	perl extracti18n.pl > i18ndata
	$(XGETTEXT) i18ndata rc.cpp `find . -name \*.cc -o -name \*.h -o -name \*.cpp` ui/kis_aboutdata.h -o $(podir)/krita.pot
	rm -f i18ndata

DOXYGEN_EXCLUDE = CImg.h colorspaces plugins kritacolor/colorspaces core/tiles
include $(top_srcdir)/admin/Doxyfile.am

kde_services_DATA = kritapart.desktop

