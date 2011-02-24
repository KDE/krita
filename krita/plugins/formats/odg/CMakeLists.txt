
include_directories( ${KRITA_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/image/metadata )

set(kritaodgimport_PART_SRCS
    kis_odg_import.cc
    )

kde4_add_plugin(kritaodgimport ${kritaodgimport_PART_SRCS})

include_directories(${PNG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(kritaodgimport kritaui ${PNG_LIBRARIES} )

install(TARGETS kritaodgimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_odg_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_odg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
