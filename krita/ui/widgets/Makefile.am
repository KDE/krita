INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../../core/ \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/tiles \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkiswidgets.la

libkiswidgets_la_SOURCES = kis_filter_configuration_widget.cc \
	kis_multi_integer_filter_widget.cc \
	kis_wdg_text_brush.ui \
	kis_text_brush.cc

noinst_HEADERS = kis_multi_integer_filter_widget.h \
	kis_filter_configuration_widget.h \
	kis_text_brush.h

libkiswidgets_la_METASOURCES = AUTO
