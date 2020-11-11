add_subdirectory(tests)
add_subdirectory(brushes)

include_directories(${LIBMYPAINT_INCLUDE_DIR})
link_directories(${LIBMYPAINT_LIBRARIES})

set(kritamypaintop_SOURCES
    my_paintop_plugin.cpp
    kis_my_paintop.cpp
    kis_my_paintop_option.cpp
    kis_my_paintop_settings.cpp
    kis_my_paintop_settings_widget.cpp
    kis_mypaint_surface.cpp
    kis_mypaint_brush.cpp
    kis_my_paintop_factory.cpp
    kis_mypaint_curve_option_widget.cpp
    kis_mypaintbrush_option.cpp
    kis_mypaint_curve_option.cpp
    ../libpaintop/kis_multi_sensors_selector.cpp
    ../libpaintop/kis_multi_sensors_model_p.cpp
)

ki18n_wrap_ui(kritamypaintop_SOURCES wdgmypaintoptions.ui wdgmypaintcurveoption.ui wdgmypaintbrushoptionsselector.ui)
#ki18n_wrap_ui(kritamypaintop_SOURCES wdgmypaintcurveoption.ui )

add_library(kritamypaintop MODULE ${kritamypaintop_SOURCES})

target_link_libraries(kritamypaintop kritalibpaintop mypaint kritawidgetutils kritaui kritalibbrush)
#target_link_libraries(kritamypaintop kritaui kritalibbrush kritawidgetutils)
#target_link_libraries(kritamypaintop LINK_INTERFACE_LIBRARIES kritaui kritalibbrush)


install(TARGETS kritamypaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita-mypaint.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
