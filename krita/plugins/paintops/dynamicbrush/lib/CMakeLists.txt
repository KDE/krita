include_directories(sensors ui)

set(kritadynamicbrushlib_LIB_SRCS
    kis_dynamic_brush_registry.cc
    kis_dynamic_brush.cc
    kis_dynamic_coloring.cc
    kis_dynamic_coloring_program.cc
    kis_dynamic_coloring_program_factory_registry.cc
    kis_dynamic_program.cc
    kis_dynamic_scattering.cc
    kis_dynamic_sensor.cc
    kis_dynamic_shape.cc
    kis_dynamic_shape_program.cc
    kis_dynamic_shape_program_factory_registry.cc
    shapes/kis_bristle_shape.cc
    shapes/kis_dab_shape.cc
    sensors/kis_dynamic_sensors.cc
    sensors/kis_dynamic_sensor_time.cc
    ui/kis_sensor_selector.cc
)

kde4_add_ui_files(kritadynamicbrushlib_LIB_SRCS
    ui/SensorSelector.ui
    sensors/SensorTimeConfiguration.ui )

kde4_add_library(kritadynamicbrush SHARED ${kritadynamicbrushlib_LIB_SRCS})

target_link_libraries(kritadynamicbrush kritaui )

set_target_properties(kritadynamicbrush PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritadynamicbrush  DESTINATION ${LIB_INSTALL_DIR})

