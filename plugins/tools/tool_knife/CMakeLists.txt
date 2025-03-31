set(kritatoolKnife_SOURCES
    ToolKnife.cpp
    KisToolKnife.cpp
    KisToolKnifeOptionsWidget.cpp
	CutThroughShapeStrategy.cpp
    RemoveGutterStrategy.cpp
    GutterWidthsConfig.cpp
    )

ki18n_wrap_ui(kritatoolKnife_SOURCES KisToolKnifeOptionsWidget.ui)

kis_add_library(kritatoolKnife MODULE ${kritatoolKnife_SOURCES})

generate_export_header(kritatoolKnife BASE_NAME kritatoolKnife)

target_link_libraries(kritatoolKnife kritaui)

install(TARGETS kritatoolKnife  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

