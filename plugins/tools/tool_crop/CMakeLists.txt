set(kritatoolcrop_SOURCES
    tool_crop.cc
    kis_tool_crop.cc
    kis_constrained_rect.cpp
    kistoolcropconfigwidget.cpp
    )
qt5_add_resources(kritatoolcrop_SOURCES toolcrop.qrc)

ki18n_wrap_ui(kritatoolcrop_SOURCES wdg_tool_crop.ui)

kis_add_library(kritatoolcrop MODULE ${kritatoolcrop_SOURCES})

target_link_libraries(kritatoolcrop kritaui)

install(TARGETS kritatoolcrop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES
    KisToolCrop.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
