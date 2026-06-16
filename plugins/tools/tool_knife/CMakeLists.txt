add_subdirectory(tests)

set(kritatoolKnife_static_SOURCES
    KisToolKnife.cpp
    KisToolKnifeOptionsWidget.cpp
    CutThroughShapeStrategy.cpp
    RemoveGutterStrategy.cpp
    GutterWidthsConfig.cpp
    )


set(kritatoolKnife_SOURCES
    ToolKnife.cpp
    )


ki18n_wrap_ui(kritatoolKnife_static_SOURCES KisToolKnifeOptionsWidget.ui)

kis_add_library(kritatoolKnife_static STATIC ${kritatoolKnife_static_SOURCES})
generate_export_header(kritatoolKnife_static BASE_NAME kritatoolKnife)


kis_add_library(kritatoolKnife MODULE ${kritatoolKnife_SOURCES})


target_link_libraries(kritatoolKnife_static PUBLIC kritaui)
target_link_libraries(kritatoolKnife PRIVATE kritatoolKnife_static)

install(TARGETS kritatoolKnife  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

