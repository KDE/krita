add_subdirectory(tests)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(kritaxcfimport_PART_SRCS
    kis_xcf_import.cpp
    xcf-general.c
    utils.c
    enums.c
    pixels.c
    scaletab.c
    table.c
    )

kde4_add_plugin(kritaxcfimport ${kritaxcfimport_PART_SRCS})

target_link_libraries(kritaxcfimport kritaui )

install(TARGETS kritaxcfimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  krita_xcf_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_xcf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
