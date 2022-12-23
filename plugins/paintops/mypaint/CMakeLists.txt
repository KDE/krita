add_subdirectory(tests)
add_subdirectory(brushes)

include_directories(${LIBMYPAINT_INCLUDE_DIR})
link_directories(${LIBMYPAINT_LIBRARIES})

set(kritamypaintop_SOURCES
    MyPaintSensorPack.cpp
    MyPaintCurveOptionData.cpp
    MyPaintCurveOptionWidget2.cpp
    MyPaintCurveRangeModel.cpp
    MyPaintCurveOptionRangeControlsStrategy.cpp
    MyPaintPaintOp.cpp
    MyPaintBasicOptionData.cpp
    MyPaintBasicOptionModel.cpp
    MyPaintBasicOptionWidget.cpp
    MyPaintPaintOpSettings.cpp
    MyPaintPaintOpSettingsWidget.cpp
    MyPaintSurface.cpp
    MyPaintPaintOpPreset.cpp
    MyPaintPaintOpFactory.cpp
    MyPaintStandardOptionData.cpp
)

ki18n_wrap_ui(kritamypaintop_SOURCES wdgmypaintoptions.ui wdgmypaintcurveoption.ui)

kis_add_library(kritamypaintop_static STATIC ${kritamypaintop_SOURCES})

target_link_libraries(kritamypaintop_static kritalibpaintop mypaint kritawidgetutils kritaui kritalibbrush kritaresources)

kis_add_library(kritamypaintop MODULE MyPaintPaintOpPlugin.cpp)

target_link_libraries(kritamypaintop kritamypaintop_static)

install(TARGETS kritamypaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita-mypaint.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
