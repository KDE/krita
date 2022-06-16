set(KRITA_smallCOLORSELECTOR_SOURCES
    smallcolorselector.cc
    smallcolorselector_dock.cc
    kis_small_color_widget.cc
    KisGLImageF16.cpp
    KisGLImageWidget.cpp
    KisClickableGLImageWidget.cpp
    )

set(small_color_selector_QRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kis_gl_image_widget.qrc
    CACHE INTERNAL "small_color_selector_QRCS"
)
qt5_add_resources(KRITA_smallCOLORSELECTOR_SOURCES ${small_color_selector_QRCS})

add_library(kritasmallcolorselector MODULE ${KRITA_smallCOLORSELECTOR_SOURCES})
target_link_libraries(kritasmallcolorselector kritaui)
install(TARGETS kritasmallcolorselector  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
