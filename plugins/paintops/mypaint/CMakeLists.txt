add_subdirectory(tests)
add_subdirectory(brushes)

if(LIBMYPAINT_FOUND)
    include_directories(${LIBMYPAINT_INCLUDE_DIR})
    link_directories(${LIBMYPAINT_LIBRARIES})
endif()

set(kritamypaintop_SOURCES
    my_paintop_plugin.cpp
    kis_my_paintop.cpp
    kis_my_paintop_option.cpp
    kis_my_paintop_settings.cpp
    kis_my_paintop_settings_widget.cpp
    kis_mypaint_surface.cpp
    kis_mypaint_brush.cpp
    kis_my_paintop_factory.cpp
)

#ki18n_wrap_ui(kritasketchpaintop_SOURCES wdgsketchoptions.ui )

add_library(kritamypaintop MODULE ${kritamypaintop_SOURCES})

target_link_libraries(kritamypaintop kritalibpaintop mypaint)

install(TARGETS kritamypaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita-mypaint.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
