
set(MAPS_PLUG koreport_mapsplugin)

find_package(Marble REQUIRED)

include_directories(${MARBLE_INCLUDE_DIR})

include_directories( ${KOMAIN_INCLUDES}
    ${KOODF_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/
    ${CMAKE_SOURCE_DIR}/libs/koreport
    ${CMAKE_SOURCE_DIR}/libs/koreport/common
    ${CMAKE_SOURCE_DIR}/libs/koreport/renderer
    ${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
    ${CMAKE_SOURCE_DIR}/plugins/chartshape/kdchart/include
)

#build a shared library
set(koreport_mapsplugin_LIB_SRCS
    KoReportDesignerItemMaps.cpp
    KoReportItemMaps.cpp
    KoReportMapsPlugin.cpp
    krscriptmaps.cpp
)

kde4_add_plugin(${MAPS_PLUG} ${koreport_mapsplugin_LIB_SRCS})

target_link_libraries(${MAPS_PLUG} ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} koreport ${KOPROPERTY_LIBS} ${MARBLE_LIBRARIES})


########### install files ###############

install(TARGETS ${MAPS_PLUG}  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  koreport_mapsplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons)
