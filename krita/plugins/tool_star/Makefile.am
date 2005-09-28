kde_services_DATA = kritatoolstar.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../tools \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

kritatoolstar_la_SOURCES = \
	wdg_tool_star.ui \
        tool_star.cc \
	kis_tool_star.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolstar.la

noinst_HEADERS = \
	tool_star.h \
	kis_tool_star.h 

kritatoolstar_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolstar_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatoolstar_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
