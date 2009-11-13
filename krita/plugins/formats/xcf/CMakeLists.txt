# add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_CURRENT_fSOURCE_DIR} )

set(kritaxcfimport_PART_SRCS
    kis_xcf_import.cpp
    xcf-general.c
    utils.c
    enums.c
    )

kde4_add_plugin(kritaxcfimport WITH_PREFIX ${kritaxcfimport_PART_SRCS})

target_link_libraries(kritaxcfimport kritaui )

install(TARGETS kritaxcfimport  DESTINATION ${PLUGIN_INSTALL_DIR})
