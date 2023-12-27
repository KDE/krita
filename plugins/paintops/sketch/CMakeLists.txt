set(kritasketchpaintop_SOURCES
    sketch_paintop_plugin.cpp
    kis_sketch_paintop.cpp
    kis_sketch_paintop_settings.cpp
    kis_sketch_paintop_settings_widget.cpp
    KisSketchOpOptionData.cpp
    KisSketchOpOptionModel.cpp
    KisSketchOpOptionWidget.cpp
    )

ki18n_wrap_ui(kritasketchpaintop_SOURCES wdgsketchoptions.ui )

kis_add_library(kritasketchpaintop MODULE ${kritasketchpaintop_SOURCES})

target_link_libraries(kritasketchpaintop kritalibpaintop)

install(TARGETS kritasketchpaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita-sketch.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


