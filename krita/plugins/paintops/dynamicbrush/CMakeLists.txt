include_directories(programs/filterslist lib/ lib/transformations lib/ui lib/shapes/ ../libpaintop)
add_subdirectory(lib)
add_subdirectory(programs)

set(kritadynamicbrush_PART_SRCS
    dynamicbrush.cc
    kis_dynamicop.cc
    kis_dynamicop_factory.cpp
    kis_dynamicop_settings.cpp
    kis_dynamicop_settings_widget.cpp
    kis_dynamic_programs_editor.cc
    )

kde4_add_ui_files(kritadynamicbrush_PART_SRCS DynamicBrushOptions.ui DynamicProgramsEditor.ui)

kde4_add_plugin(kritadynamicbrushpaintop ${kritadynamicbrush_PART_SRCS})

target_link_libraries(kritadynamicbrushpaintop kritalibpaintop kritadynamicbrush
  # link to this to "load" the programs factory
  #kritafilterslistdynamicprogram
  kritabasicdynamicshapeprogram
  kritabasicdynamiccoloringprogram
  )

install(TARGETS kritadynamicbrushpaintop DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  kritadynamicbrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  dynamicbrush.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES krita-dynamic.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

