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
	kis_curve_framework.cc \
	kis_tool_curve.cc \
	wdg_tool_example.ui \
        tool_example.cc \
	kis_tool_example.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolcurves.la

noinst_HEADERS = \
	kis_curve_framework.h \
	kis_tool_curve.h \
	tool_example.h \
	kis_tool_example.h

kritatoolcurves_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolcurves_la_LIBADD =  ../../../libkritacommon.la

kritatoolcurves_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

kritapics_DATA = \
	tool_example.png \
	tool_example_cursor.png

kritapicsdir = $(kde_datadir)/krita/pics

