
set(krosskritacore_LIB_SRCS
   kritacoremodule.cpp
   kritacoreprogress.cpp
   krs_paint_layer.cpp
   krs_image.cpp
   krs_histogram.cpp
   krs_painter.cpp
   krs_color.cpp
   krs_brush.cpp
   krs_pattern.cpp
   krs_filter.cpp
   krs_wavelet.cpp
   )

kde4_automoc(${krosskritacore_LIB_SRCS})

qt4_wrap_cpp(krosskritacore_LIB_SRCS krs_iterator.h)

kde4_add_library(krosskritacore SHARED ${krosskritacore_LIB_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosskritacore )

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(krosskritacore kritaui)

install(TARGETS krosskritacore DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############
