kde_services_DATA = kritatooltransform.desktop

# all_includes must remain last!
INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../core \
	-I$(srcdir)/../../core/color_strategy/ \
	-I$(srcdir)/../../core/resources/ \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../ui/dialogs \
	-I$(srcdir)/../../ui/widgets \
	-I../../ui/dialogs \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(srcdir)/../../core/strategy/ $(all_includes)

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
kritatooltransform_la_LIBADD =  ../../libkritacommon.la $(LIB_KOPAINTER) $(LIB_KOFFICECORE)

kritatooltransform_la_METASOURCES = AUTO

KDE_OPTIONS = nofinal
