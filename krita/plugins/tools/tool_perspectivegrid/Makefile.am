kde_services_DATA = kritatoolperspectivegrid.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritatoolperspectivegrid_la_SOURCES = \
	tool_perspectivegrid.cc \
	kis_tool_perspectivegrid.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolperspectivegrid.la

noinst_HEADERS = \
	tool_perspectivegrid.h \
	kis_tool_perspectivegrid.h 

kritatoolperspectivegrid_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolperspectivegrid_la_LIBADD =  ../../../libkritacommon.la

kritatoolperspectivegrid_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

#kritapics_DATA = \
#	tool_perspectivegrid_cursor.png

kritapicsdir = $(kde_datadir)/krita/pics

