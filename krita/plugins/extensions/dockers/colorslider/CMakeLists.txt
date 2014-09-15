set(kritacolorslider_PART_SRCS
    kis_color_slider.cpp
    kis_color_slider_dock.cpp
    kis_color_slider_widget.cpp
    kis_color_slider_input.cpp
    kis_color_slider_input.cpp
    kis_hsv_slider.cpp

)

# kde4_add_ui_files(kritaspecificcolorselector_PART_SRCS myuifile.ui )

kde4_add_plugin(kritacolorslider ${kritacolorslider_PART_SRCS})

target_link_libraries(kritacolorslider kritaui ${OPENEXR_LIBRARIES})

install(TARGETS kritacolorslider  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_colorslider.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
