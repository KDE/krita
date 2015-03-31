
set(kritaconvolutionfilters_PART_SRCS
   convolutionfilters.cpp
   kis_convolution_filter.cpp )


add_library(kritaconvolutionfilters MODULE ${kritaconvolutionfilters_PART_SRCS})



target_link_libraries(kritaconvolutionfilters kritaui)

install(TARGETS kritaconvolutionfilters  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaconvolutionfilters.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

