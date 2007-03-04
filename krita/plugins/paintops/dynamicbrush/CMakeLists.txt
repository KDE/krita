include_directories(programs/filterslist lib/ lib/transformations )
add_subdirectory(lib)
add_subdirectory(programs)

set(kritadynamicbrush_PART_SRCS
    dynamicbrush.cc
    kis_dynamicop.cc
    kis_dynamic_programs_editor.cc
    )

kde4_add_ui_files(kritadynamicbrush_PART_SRCS DynamicBrushOptions.ui DynamicProgramsEditor.ui)
kde4_automoc(${kritadynamicbrush_PART_SRCS})

kde4_add_plugin(kritadynamicbrushpaintop ${kritadynamicbrush_PART_SRCS})


target_link_libraries(kritadynamicbrushpaintop kritaui kritadynamicbrush kritafilterslistdynamicprogram )

install(TARGETS kritadynamicbrushpaintop DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadynamicbrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  dynamicbrush.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)

