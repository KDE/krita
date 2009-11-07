macro_optional_find_package(GLIB2)

macro_log_feature(GLIB2_FOUND "GLIB2" "GLib 2.0" "http://gtk.org" FALSE
"" "Required by the Krita MyPaint-compatible brush engine")


if(GLIB2_FOUND)

include_directories(brushlib ${GLIB2_INCLUDE_DIR})

set(kritamypaintpaintop_PART_SRCS
    mypaint_brush_resource.cpp
    mypaint_paintop.cpp
    mypaint_paintop_factory.cpp
    mypaint_paintop_plugin.cpp
    mypaint_paintop_settings.cpp
    mypaint_paintop_settings_widget.cpp
    mypaint_surface.cpp
    mybrush_resources_listmodel.cpp
)

kde4_add_ui_files(kritamypaintpaintop_PART_SRCS wdgmypaintoptions.ui )

kde4_add_plugin(kritamypaintpaintop ${kritamypaintpaintop_PART_SRCS})

target_link_libraries(kritamypaintpaintop kritaui ${GLIB2_LIBRARIES})

install( TARGETS kritamypaintpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritamypaintpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES krita-mypaint.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

add_subdirectory(brushes)

endif(GLIB2_FOUND)
