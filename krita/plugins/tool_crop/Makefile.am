kde_services_DATA = kritatoolcrop.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

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
kritatoolcrop_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatoolcrop_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
