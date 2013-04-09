
set(MAPS_PLUG koreport_mapsplugin)

include_directories(
    ${MARBLE_INCLUDE_DIR}
    ${KORDF_INCLUDES}
    ${KOPROPERTY_INCLUDES}
    ${KOREPORT_INCLUDES}
)

#build a shared library
set(koreport_mapsplugin_LIB_SRCS
    KoReportDesignerItemMaps.cpp
    KoReportItemMaps.cpp
    KoReportMapsPlugin.cpp
    krscriptmaps.cpp
)

kde4_add_plugin(${MAPS_PLUG} ${koreport_mapsplugin_LIB_SRCS})

target_link_libraries(${MAPS_PLUG} ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kordf ${KOREPORT_LIBS} ${KOPROPERTY_LIBS} ${MARBLE_LIBRARIES})


########### install files ###############

install(TARGETS ${MAPS_PLUG}  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  koreport_mapsplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons)
