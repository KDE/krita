INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../../core/ \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/tiles \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkiswidgets.la

libkiswidgets_la_SOURCES = kis_filter_configuration_widget.cc \
	kis_multi_integer_filter_widget.cc \
	kis_gradient_slider_widget.cc \
	kis_wdg_autogradient.ui \
	kis_autogradient.cc \
	kis_wdg_text_brush.ui \
	kis_text_brush.cc \
	kis_matrix_widget.ui

noinst_HEADERS = kis_multi_integer_filter_widget.h \
	kis_filter_configuration_widget.h \
	kis_gradient_slider_widget.h \
	kis_autogradient.h \
	kis_text_brush.h

libkiswidgets_la_METASOURCES = AUTO
