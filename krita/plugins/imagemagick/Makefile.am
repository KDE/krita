kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritamagick.rc
EXTRA_DIST = $(kritarc_DATA)

kde_services_DATA = kritamagick.desktop

# all_includes must remain last!
INCLUDES = -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/filters \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../core/builder \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/dialogs \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/dialogs \
	-I$(srcdir)/../../core/strategy/ \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES)  $(LIBMAGICK_CPPFLAGS)  $(all_includes)

kritamagick_la_SOURCES = \
	kritamagick.cc \
	kis_magick_converter.cc 
#	kis_magick_filters.cc \
#	kis_magick_impexp.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritamagick.la

noinst_HEADERS = \
	kritamagick.h \
	kis_magick_converter.h 
# 	kis_magick_filters.h \
# 	kis_magick_impexp.h

kritamagick_la_LDFLAGS = $(all_libraries) $(LIBMAGICK_LDFLAGS) $(KDE_RPATH) $(LIBMAGICK_RPATH) -module $(KDE_PLUGIN)
kritamagick_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE) $(LIBMAGICK_LIBS)

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

kritamagick_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
