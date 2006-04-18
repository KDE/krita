kde4_header()




########### next target ###############

set(kritaconvolutionfilters_PART_SRCS 
   kis_custom_convolution_filter_configuration_widget.cc 
   kis_custom_convolution_filter.cc 
   convolutionfilters.cc 
   kis_convolution_filter.cc )

kde4_automoc(${kritaconvolutionfilters_PART_SRCS})

kde4_add_ui3_files(kritaconvolutionfilters_PART_SRCS kis_custom_convolution_filter_configuration_base_widget.ui )

kde4_add_plugin(kritaconvolutionfilters ${kritaconvolutionfilters_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaconvolutionfilters )

target_link_libraries(kritaconvolutionfilters  ${KDE4_KDECORE_LIBS} kritacommon )

install_targets(${PLUGIN_INSTALL_DIR} kritaconvolutionfilters )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritaconvolutionfilters.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = kritaconvolutionfilters.desktop
#
#INCLUDES  = \
#	 -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../ui \
#	-I../../../ui/ \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kritaconvolutionfilters.la
#
#kritaconvolutionfilters_la_SOURCES = kis_custom_convolution_filter_configuration_base_widget.ui \
#	kis_custom_convolution_filter_configuration_widget.cc \
#	kis_custom_convolution_filter.cc \
#	convolutionfilters.cc \
#	kis_convolution_filter.cc
#
#noinst_HEADERS = convolutionfilters.h \
#	kis_custom_convolution_filter_configuration_widget.h \
#	kis_convolution_filter.h
#
#kritaconvolutionfilters_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kritaconvolutionfilters_la_LIBADD =  ../../../libkritacommon.la
#
#kritaconvolutionfilters_la_METASOURCES = AUTO
