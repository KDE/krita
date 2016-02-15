set(kritaconvolutionfilters_SOURCES
   convolutionfilters.cpp
   kis_convolution_filter.cpp )
add_library(kritaconvolutionfilters MODULE ${kritaconvolutionfilters_SOURCES})
target_link_libraries(kritaconvolutionfilters kritaui)
install(TARGETS kritaconvolutionfilters  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
