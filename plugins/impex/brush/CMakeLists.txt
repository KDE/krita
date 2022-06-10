add_subdirectory(tests)

set(kritabrushexport_PART_SRCS
    kis_brush_export.cpp
    KisWdgOptionsBrush.cpp
    KisAnimatedBrushAnnotation.cpp
    )

ki18n_wrap_ui(kritabrushexport_PART_SRCS  wdg_export_gih.ui)

kis_add_library(kritabrushexport MODULE ${kritabrushexport_PART_SRCS})

target_link_libraries(kritabrushexport kritalibbrush kritalibpaintop kritaui kritaimpex)

install(TARGETS kritabrushexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritabrushimport_PART_SRCS
    kis_brush_import.cpp
    KisAnimatedBrushAnnotation.cpp
    )

ki18n_wrap_ui(kritabrushimport_PART_SRCS )

kis_add_library(kritabrushimport MODULE ${kritabrushimport_PART_SRCS})

target_link_libraries(kritabrushimport kritalibbrush kritaui)

install(TARGETS kritabrushimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_brush.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
