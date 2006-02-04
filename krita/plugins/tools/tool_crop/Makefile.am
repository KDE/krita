kde_services_DATA = kritatoolcrop.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../ui \
	-I$/../../../ui \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kritatoolcrop_la_SOURCES = \
	wdg_tool_crop.ui \
	tool_crop.cc \
	kis_tool_crop.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolcrop.la

noinst_HEADERS = \
	tool_crop.h \
	kis_tool_crop.h 

kritatoolcrop_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolcrop_la_LIBADD =  ../../../libkritacommon.la

kritatoolcrop_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal

kritapics_DATA = \
	tool_crop_cursor.png

kritapicsdir = $(kde_datadir)/krita/pics

