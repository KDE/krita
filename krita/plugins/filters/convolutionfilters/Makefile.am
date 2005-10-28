kde_services_DATA = kritaconvolutionfilters.desktop

INCLUDES  = \
	 -I$(srcdir)/../../../sdk \
	-I$(srcdir)/../../../kritacolor/ \
	-I$(srcdir)/../../../core \
	-I$(srcdir)/../../../ui \
	-I../../../ui/ \
	$(KOFFICE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kritaconvolutionfilters.la

kritaconvolutionfilters_la_SOURCES = kis_custom_convolution_filter_configuration_base_widget.ui \
	kis_custom_convolution_filter_configuration_widget.cc \
	kis_custom_convolution_filter.cc \
	convolutionfilters.cc \
	kis_convolution_filter.cc

noinst_HEADERS = convolutionfilters.h \
	kis_custom_convolution_filter_configuration_widget.h \
	kis_convolution_filter.h

kritaconvolutionfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaconvolutionfilters_la_LIBADD =  ../../../libkritacommon.la

kritaconvolutionfilters_la_METASOURCES = AUTO
