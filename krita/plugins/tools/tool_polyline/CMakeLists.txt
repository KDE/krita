

set(kritatoolpolyline_PART_SRCS
    tool_polyline.cc
    kis_tool_polyline.cc
    )


add_library(kritatoolpolyline MODULE ${kritatoolpolyline_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolpolyline kritatoolpolyline.desktop)


target_link_libraries(kritatoolpolyline kritaui)

install(TARGETS kritatoolpolyline  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_polyline_cursor.png  	polyline.png dark_polyline.png light_polyline.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolpolyline.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


install( FILES
    KisToolPolyline.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
