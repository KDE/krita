include_directories(${KOKROSS_INCLUDES})

set(krossmodulekrita_LIB_SRCS
   krs_brush.cpp
   krs_color.cpp
   krs_const_paint_device.cpp
   krs_filter.cpp
   krs_histogram.cpp
   krs_image.cpp
   krs_paint_device.cpp
   krs_paint_layer.cpp
   krs_pattern.cpp
   krs_painter.cpp
   krs_wavelet.cpp
   krs_module.cpp
   krs_progress.cpp
   krs_monitor.cpp
   )

qt4_wrap_cpp(krossmodulekrita_LIB_SRCS krs_iterator.h krs_const_iterator.h)
kde4_add_library(krossmodulekrita SHARED ${krossmodulekrita_LIB_SRCS})
##add_definitions(${KDE4_ENABLE_EXCEPTIONS})
target_link_libraries(krossmodulekrita kokross kritaui)
set_target_properties(krossmodulekrita PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krossmodulekrita DESTINATION ${LIB_INSTALL_DIR})
