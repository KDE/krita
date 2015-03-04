add_subdirectory(tests)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-undef -Wno-missing-format-attribute -Wno-sign-compare)
endif ()

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

if (WIN32)
    target_link_libraries(kritaxcfimport kritaui ${WIN32_PLATFORM_NET_LIBS})
else ()
    install(TARGETS kritaxcfimport  DESTINATION ${PLUGIN_INSTALL_DIR})
endif ()

########### install files ###############

install( FILES  krita_xcf_import.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_xcf.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
