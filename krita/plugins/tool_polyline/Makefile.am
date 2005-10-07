kde_services_DATA = kritatoolpolyline.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../ui \
        -I$(top_srcdir)/lib/kofficecore \
        -I$(top_srcdir)/lib/kofficeui \
	$(all_includes)

kritatoolpolyline_la_SOURCES = \
        tool_polyline.cc \
	kis_tool_polyline.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatoolpolyline.la

noinst_HEADERS = \
	tool_polyline.h \
	kis_tool_polyline.h 

kritatoolpolyline_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatoolpolyline_la_LIBADD =  ../../libkritacommon.la 

kritatoolpolyline_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
