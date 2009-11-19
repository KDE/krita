add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_CURRENT_fSOURCE_DIR} )

set(kritaxcfimport_PART_SRCS
    kis_xcf_import.cpp
    xcf-general.c
    utils.c
    enums.c
    pixels.c
    scaletab.c
    table.c
    )

kde4_add_plugin(kritaxcfimport WITH_PREFIX ${kritaxcfimport_PART_SRCS})

target_link_libraries(kritaxcfimport kritaui )

install(TARGETS kritaxcfimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  krita_xcf_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_xcf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
