AM_CXXFLAGS = -DQT_PLUGIN

INCLUDES =  -I$(srcdir)/../../sdk \
	 -I$(srcdir)/../../ui \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/tool \
	$(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) $(all_includes)

kde_widget_LTLIBRARIES = kritawidgets.la

METASOURCES = AUTO

kritawidgets_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kritawidgets_la_LIBADD = $(srcdir)/../../libkritacommon.la \
	$(LIB_KDECORE)

kritawidgets_la_SOURCES = kis_widgets_plugin.cc
