
########### Flake Plugin library ###############
project (MapShape)

include_directories( ${MARBLE_INCLUDE_DIR}
                     ${KOODF_INCLUDES}
                     ${CMAKE_SOURCE_DIR}
                     ${CMAKE_BINARY_DIR}
                     ${CALLIGRA_INCLUDE_DIR}
                   )

SET (mapsshape_LIB_SRCS
    MapShape.cpp
    MapShapeFactory.cpp
    MapTool.cpp
    MapToolFactory.cpp
    Plugin.cpp
    MapShapeCommandChangeProjection.cpp
    MapShapeCommandContentChange.cpp
    MapShapeCommandSetMapThemeId.cpp
    MapShapeCommandZoom.cpp
)

kde4_add_plugin(mapshape ${mapsshape_LIB_SRCS})

target_link_libraries(mapshape komain ${MARBLE_LIBRARIES})
install(TARGETS mapshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES mapshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons(${DATA_INSTALL_DIR}/calligra/icons)
