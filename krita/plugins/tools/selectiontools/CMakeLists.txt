

set(kritaselectiontools_PART_SRCS
    selection_tools.cc
    kis_tool_select_rectangular.cc
    kis_tool_select_polygonal.cc
    kis_tool_select_elliptical.cc
    kis_tool_select_contiguous.cc
    kis_tool_select_outline.cc
    kis_tool_select_path.cc
    kis_tool_select_similar.cc
)

add_library(kritaselectiontools MODULE ${kritaselectiontools_PART_SRCS})
kcoreaddons_desktop_to_json(kritaselectiontools kritaselectiontools.desktop)

target_link_libraries(kritaselectiontools kritaui basicflakes)

install(TARGETS kritaselectiontools  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_path_selection.png
                tool_rect_selection.png
                tool_eraser_selection.png
                tool_contiguous_selection.png
                tool_elliptical_selection.png
                tool_outline_selection.png
                tool_polygonal_selection.png
                tool_rectangular_selection_cursor.png
                tool_eraser_selection_cursor.png
                tool_contiguous_selection_cursor.png
                tool_elliptical_selection_cursor.png
                tool_outline_selection_cursor.png
                tool_polygonal_selection_cursor.png
                tool_similar_selection.png
                tool_similar_selection_cursor.png
                
                dark_tool_contiguous_selection.png
                dark_tool_outline_selection.png
                dark_tool_path_selection.png
                dark_tool_polygonal_selection.png
                dark_tool_elliptical_selection.png
                dark_tool_rect_selection.png
                dark_tool_similar_selection.png

                light_tool_contiguous_selection.png
                light_tool_outline_selection.png
                light_tool_path_selection.png
                light_tool_polygonal_selection.png
                light_tool_elliptical_selection.png
                light_tool_rect_selection.png
                light_tool_similar_selection.png
                DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritaselectiontools.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


install( FILES
    KisToolSelectPolygonal.action
    KisToolSelectElliptical.action
    KisToolSelectSimilar.action
    KisToolSelectContiguous.action
    KisToolSelectRectangular.action
    KisToolSelectOutline.action
    KisToolSelectPath.action

DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
