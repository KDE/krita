set(kritaconvolutionfilters_SOURCES
   convolutionfilters.cpp
   kis_convolution_filter.cpp )
kis_add_library(kritaconvolutionfilters MODULE ${kritaconvolutionfilters_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritaconvolutionfilters kritaui)
install(TARGETS kritaconvolutionfilters  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
