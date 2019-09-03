set(kritaselectiontools_SOURCES
    selection_tools.cc
    kis_tool_select_rectangular.cc
    kis_tool_select_polygonal.cc
    kis_tool_select_elliptical.cc
    kis_tool_select_contiguous.cc
    kis_tool_select_outline.cc
    kis_tool_select_path.cc
    kis_tool_select_similar.cc
    kis_selection_modifier_mapper.cc
)

qt5_add_resources(kritaselectiontools_SOURCES selectiontools.qrc)

kis_add_library(kritaselectiontools MODULE ${kritaselectiontools_SOURCES})

target_link_libraries(kritaselectiontools kritaui kritabasicflakes)

install(TARGETS kritaselectiontools  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES
        KisToolSelectPolygonal.action
        KisToolSelectElliptical.action
        KisToolSelectSimilar.action
        KisToolSelectContiguous.action
        KisToolSelectRectangular.action
        KisToolSelectOutline.action
        KisToolSelectPath.action
    DESTINATION  ${DATA_INSTALL_DIR}/krita/actions
)
