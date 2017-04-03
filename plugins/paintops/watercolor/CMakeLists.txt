set(kritawatercolorpaintop_SOURCES
    kis_splat.cpp
    kis_wetmap.cpp
    kis_splat_generator.cpp
    )

add_library(kritawatercolorpaintop MODULE ${kritawatercolorpaintop_SOURCES})

target_link_libraries(kritawatercolorpaintop kritalibpaintop)

install(TARGETS kritawatercolorpaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
