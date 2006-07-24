kde_services_DATA = kritatoolsiox.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritatoolsiox_la_SOURCES = \
	wdg_tool_siox.ui \
        tool_siox.cc \
	kis_tool_siox.cc \
	cielab.cc \
	siox.cc

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolsiox.la

noinst_HEADERS = \
	tool_siox.h \
	kis_tool_siox.h \
	cielab.h \
	siox.h

kritatoolsiox_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolsiox_la_LIBADD =  ../../../libkritacommon.la

kritatoolsiox_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

kritapics_DATA = \
	tool_siox.png \
	tool_siox_cursor.png

kritapicsdir = $(kde_datadir)/krita/pics

