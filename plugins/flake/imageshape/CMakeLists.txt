project(imageshape)

set ( ImageShape_SRCS
    ImageShapePlugin.cpp
    ImageShape.cpp
    ImageShapeFactory.cpp
)

kis_add_library(krita_shape_image MODULE ${ImageShape_SRCS} PCH_FILE kis_global_pch.h)

target_link_libraries(krita_shape_image kritaflake kritawidgets KF5::I18n )

install(TARGETS krita_shape_image DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

