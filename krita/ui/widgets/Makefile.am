INCLUDES  = -I$(srcdir)/../ \
	-I$(srcdir)/../../core/ \
	-I$(srcdir)/../../sdk \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core/resources \
	-I$(srcdir)/../../core/filters \
	-I$(srcdir)/../../core/compositeop \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../core/tool \
	-I$(srcdir)/../../core/tiles \
	$(KOFFICE_INCLUDES) -I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

noinst_LTLIBRARIES = libkiswidgets.la

libkiswidgets_la_SOURCES = kis_multi_integer_filter_widget.cc \
	kis_multi_double_filter_widget.cc \
	kis_multi_bool_filter_widget.cc \
	kis_gradient_slider_widget.cc \
	kcurve.cc\
	kis_matrix_widget.ui \
	kis_histogram_view.cc

noinst_HEADERS = kis_multi_integer_filter_widget.h \
	kis_multi_bool_filter_widget.h \
	kis_gradient_slider_widget.h kis_histogram_view.h

libkiswidgets_la_METASOURCES = AUTO
