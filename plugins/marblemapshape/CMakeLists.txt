
########### Flake Plugin library ###############
project (MarbleMapShape)

include_directories( ${MARBLE_INCLUDE_DIR}
                     ${KOODF_INCLUDES}
                     ${CMAKE_SOURCE_DIR}
                     ${CMAKE_BINARY_DIR}
                     ${CALLIGRA_INCLUDE_DIR}
                   )

SET (marblemapsshape_LIB_SRCS
    MarbleMapShape.cpp
    MarbleMapShapeFactory.cpp
    MarbleMapTool.cpp
    MarbleMapToolFactory.cpp
    Plugin.cpp
    MarbleMapShapeCommandChangeProjection.cpp
    MarbleMapShapeCommandContentChange.cpp
    MarbleMapShapeCommandSetMapThemeId.cpp
    MarbleMapShapeCommandZoom.cpp
)

kde4_add_plugin(marblemapshape ${marblemapsshape_LIB_SRCS})

target_link_libraries(marblemapshape komain ${MARBLE_LIBRARIES})
install(TARGETS marblemapshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES marblemapshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )

