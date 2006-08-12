
set(kritaconvolutionfilters_PART_SRCS
   kis_custom_convolution_filter_configuration_widget.cc
   kis_custom_convolution_filter.cc
   convolutionfilters.cc
   kis_convolution_filter.cc )

kde4_automoc(${kritaconvolutionfilters_PART_SRCS})

kde4_add_ui_files(kritaconvolutionfilters_PART_SRCS kis_custom_convolution_filter_configuration_base_widget.ui )

kde4_add_plugin(kritaconvolutionfilters ${kritaconvolutionfilters_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaconvolutionfilters )

target_link_libraries(kritaconvolutionfilters kritaui)

install(TARGETS kritaconvolutionfilters  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaconvolutionfilters.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

