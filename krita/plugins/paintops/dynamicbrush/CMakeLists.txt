include_directories(programs/filterslist)
set(kritadynamicbrush_PART_SRCS
    dynamicbrush.cc
    kis_dynamic_shape.cc
    kis_dynamic_coloring.cc
    kis_dynamic_transformation.cc
    kis_size_transformation.cc
    kis_darken_transformation.cc
    kis_dynamic_brush_registry.cc
    kis_dynamic_brush.cc
    kis_dynamicop.cc
    programs/filterslist/kis_filters_list_dynamic_programs_editor.cc
    programs/filterslist/kis_filters_list_model.cc
    programs/filterslist/kis_filters_list_dynamic_program.cc
    )

kde4_automoc(${kritadynamicbrush_PART_SRCS})

kde4_add_ui_files(kritadynamicbrush_PART_SRCS programs/filterslist/FiltersListDynamicProgramEditor.ui )


kde4_add_plugin(kritadynamicbrush ${kritadynamicbrush_PART_SRCS})


target_link_libraries(kritadynamicbrush kritaui)

install(TARGETS kritadynamicbrush  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadynamicbrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  dynamicbrush.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)

