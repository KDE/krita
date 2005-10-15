kde_services_DATA = kritatooltransform.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../kritacolor/ \
	-I$(srcdir)/../../kritacolor/color_strategy \
	-I$(srcdir)/../../ui \
        -I$(top_srcdir)/lib/kofficecore \
        -I$(top_srcdir)/lib/kofficeui \
	$(all_includes)

kritatooltransform_la_SOURCES = \
	wdg_tool_transform.ui \
	tool_transform.cc \
	kis_tool_transform.cc 

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kritatooltransform.la

noinst_HEADERS = \
	tool_transform.h \
	kis_tool_transform.h 

kritatooltransform_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritatooltransform_la_LIBADD =  ../../libkritacommon.la

kritatooltransform_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
