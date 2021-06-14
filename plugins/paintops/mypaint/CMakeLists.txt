add_subdirectory(tests)
add_subdirectory(brushes)

include_directories(${LIBMYPAINT_INCLUDE_DIR})
link_directories(${LIBMYPAINT_LIBRARIES})

set(kritamypaintop_SOURCES
    MyPaintPaintOpPlugin.cpp
    MyPaintPaintOp.cpp
    MyPaintPaintOpOption.cpp
    MyPaintPaintOpSettings.cpp
    MyPaintPaintOpSettingsWidget.cpp
    MyPaintSurface.cpp
    MyPaintPaintOpPreset.cpp
    MyPaintPaintOpFactory.cpp
    MyPaintCurveOptionWidget.cpp
    MyPaintBrushOption.cpp
    MyPaintCurveOption.cpp
    ../libpaintop/kis_multi_sensors_selector.cpp
    ../libpaintop/kis_multi_sensors_model_p.cpp
)

ki18n_wrap_ui(kritamypaintop_SOURCES wdgmypaintoptions.ui wdgmypaintcurveoption.ui)

add_library(kritamypaintop MODULE ${kritamypaintop_SOURCES})

target_link_libraries(kritamypaintop kritalibpaintop mypaint kritawidgetutils kritaui kritalibbrush kritaresources)


install(TARGETS kritamypaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita-mypaint.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
