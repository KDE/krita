project(imageshape)

set ( ImageShape_SRCS
    ImageShapePlugin.cpp
    ImageShape.cpp
    ImageShapeFactory.cpp
)

kis_add_library(krita_shape_image MODULE ${ImageShape_SRCS})

target_link_libraries(krita_shape_image kritaflake kritawidgets KF6::I18n )

install(TARGETS krita_shape_image DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

