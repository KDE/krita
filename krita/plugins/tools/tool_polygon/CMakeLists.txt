

set(kritatoolpolygon_PART_SRCS
    tool_polygon.cc
    kis_tool_polygon.cc
    )


add_library(kritatoolpolygon MODULE ${kritatoolpolygon_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolpolygon kritatoolpolygon.desktop)


target_link_libraries(kritatoolpolygon kritaui)

install(TARGETS kritatoolpolygon  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_polygon_cursor.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES
    KisToolPolygon.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
