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

kde4_add_plugin(commentshape ${commentshape_LIB_SRCS})

target_link_libraries(commentshape komain textlayout)

install(TARGETS commentshape DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES commentshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
