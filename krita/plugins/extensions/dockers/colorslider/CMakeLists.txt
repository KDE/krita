set(kritacolorslider_SOURCES
    kis_color_slider.cpp
    kis_color_slider_dock.cpp
    kis_color_slider_widget.cpp
    kis_color_slider_input.cpp
    kis_color_slider_input.cpp
    kis_hsv_slider.cpp

)

add_library(kritacolorslider MODULE ${kritacolorslider_SOURCES})
target_link_libraries(kritacolorslider kritaui ${OPENEXR_LIBRARIES})
install(TARGETS kritacolorslider  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
