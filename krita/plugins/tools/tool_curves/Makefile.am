kde_services_DATA = kritatoolcurves.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritatoolcurves_la_SOURCES = \
	wdg_tool_curves.ui \
        tool_curves.cc \
	kis_tool_curves.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolcurves.la

noinst_HEADERS = \
	tool_curves.h \
	kis_tool_curves.h 

kritatoolcurves_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolcurves_la_LIBADD =  ../../../libkritacommon.la

kritatoolcurves_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

kritapics_DATA = \
	tool_curves.png \
	tool_curves_cursor.png

kritapicsdir = $(kde_datadir)/krita/pics

