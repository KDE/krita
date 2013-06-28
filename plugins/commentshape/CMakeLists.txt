include_directories(
    ${KOTEXT_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
    ${FLAKE_INCLUDES}
    ${KOMAIN_INCLUDES})

SET( commentshape_LIB_SRCS
     CommentShape.cpp
     CommentShapeFactory.cpp
     CommentTool.cpp
     CommentToolFactory.cpp
     InitialsCommentShape.cpp
     Plugin.cpp
   )

kde4_add_plugin(calligra_shape_comment ${commentshape_LIB_SRCS})

target_link_libraries(calligra_shape_comment komain textlayout)

install(TARGETS calligra_shape_comment DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES commentshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
