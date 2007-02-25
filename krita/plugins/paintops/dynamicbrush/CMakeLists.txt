include_directories(programs/filterslist lib/ lib/transformations)
add_subdirectory(lib)
add_subdirectory(programs)

set(kritadynamicbrush_PART_SRCS
    dynamicbrush.cc
    kis_dynamicop.cc
    )

kde4_automoc(${kritadynamicbrush_PART_SRCS})

kde4_add_plugin(kritadynamicbrush ${kritadynamicbrush_PART_SRCS})


target_link_libraries(kritadynamicbrush kritaui kritadynamicbrushlib kritafilterslistdynamicprogramlib)

install(TARGETS kritadynamicbrush  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadynamicbrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  dynamicbrush.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)

