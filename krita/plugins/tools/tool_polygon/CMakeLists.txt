

set(kritatoolpolygon_PART_SRCS
    tool_polygon.cc
    kis_tool_polygon.cc
    )


add_library(kritatoolpolygon MODULE ${kritatoolpolygon_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolpolygon kritatoolpolygon.desktop)


target_link_libraries(kritatoolpolygon kritaui)

install(TARGETS kritatoolpolygon  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_polygon_cursor.png  	tool_polygon.png dark_tool_polygon.png light_tool_polygon.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolpolygon.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

install( FILES
    KisToolPolygon.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
