set(kritacolorslider_PART_SRCS
    kis_color_slider.cpp
    kis_color_slider_dock.cpp
    kis_color_slider_widget.cpp
    kis_color_slider_input.cpp
    kis_color_slider_input.cpp
    kis_hsv_slider.cpp

)

# ki18n_wrap_ui(kritaspecificcolorselector_PART_SRCS myuifile.ui )

add_library(kritacolorslider MODULE ${kritacolorslider_PART_SRCS})
kcoreaddons_desktop_to_json(kritacolorslider krita_colorslider.desktop)

target_link_libraries(kritacolorslider kritaui ${OPENEXR_LIBRARIES})

install(TARGETS kritacolorslider  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_colorslider.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
