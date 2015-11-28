

set(kritatoolpolyline_SOURCES
    tool_polyline.cc
    kis_tool_polyline.cc
    )

qt5_add_resources(kritatoolpolyline_SOURCES polyline.qrc)

add_library(kritatoolpolyline MODULE ${kritatoolpolyline_SOURCES})
kcoreaddons_desktop_to_json(kritatoolpolyline kritatoolpolyline.desktop)

target_link_libraries(kritatoolpolyline kritaui)

install(TARGETS kritatoolpolyline  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES
    KisToolPolyline.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
