include_directories(
    ${KOTEXT_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
    ${FLAKE_INCLUDES}
)

set( commentshape_LIB_SRCS
     CommentShape.cpp
     CommentShapeFactory.cpp
     CommentTool.cpp
     CommentToolFactory.cpp
     InitialsCommentShape.cpp
     Plugin.cpp
   )

kde4_add_plugin(calligra_shape_comment ${commentshape_LIB_SRCS})

target_link_libraries(calligra_shape_comment kotextlayout)

install(TARGETS calligra_shape_comment DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES calligra_shape_comment.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

# kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
