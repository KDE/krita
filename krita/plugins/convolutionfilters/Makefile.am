kritarcdir = $(kde_datadir)/krita/kpartplugins
kritarc_DATA = kritaconvolutionfilters.rc

EXTRA_DIST = $(kritarc_DATA)


INCLUDES  = \
	-I$(srcdir)/../../core/color_strategy \
	-I$(srcdir)/../../core \
	-I$(srcdir)/../../core/tiles \
	-I$(srcdir)/../../core/paintop \
	-I$(srcdir)/../../ui \
	-I$(srcdir)/../../tools \
	$(KOFFICE_INCLUDES) \
	-I$(interfacedir) $(KOPAINTER_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kritaconvolutionfilters.la

kritaconvolutionfilters_la_SOURCES = convolutionfilters.cc \
	kis_convolution_filter.cc

noinst_HEADERS = convolutionfilters.h \
	kis_convolution_filter.h

kritaconvolutionfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kritaconvolutionfilters_la_LIBADD =  ../../libkritacommon.la $(LIB_KOFFICECORE) $(LIB_KOPAINTER)

kritaconvolutionfilters_la_METASOURCES = AUTO
