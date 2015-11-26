add_subdirectory(tests)

set(libkritatiffconverter_LIB_SRCS
    kis_tiff_converter.cc
    kis_tiff_writer_visitor.cpp
    kis_tiff_reader.cc
    kis_tiff_ycbcr_reader.cc
    kis_buffer_stream.cc
    )

########## next target ###############

set(kritatiffimport_SOURCES
    ${libkritatiffconverter_LIB_SRCS}
    kis_tiff_import.cc
    )


add_library(kritatiffimport MODULE ${kritatiffimport_SOURCES})
kcoreaddons_desktop_to_json(kritatiffimport krita_tiff_import.desktop)

target_link_libraries(kritatiffimport kritaui  ${TIFF_LIBRARIES})

install(TARGETS kritatiffimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### next target ###############

set(kritatiffexport_SOURCES
    ${libkritatiffconverter_LIB_SRCS}
    kis_tiff_export.cc
    kis_dlg_options_tiff.cpp
    )

ki18n_wrap_ui(kritatiffexport_SOURCES kis_wdg_options_tiff.ui )

add_library(kritatiffexport MODULE ${kritatiffexport_SOURCES})
kcoreaddons_desktop_to_json(kritatiffexport krita_tiff_export.desktop)


target_link_libraries(kritatiffexport kritaui  ${TIFF_LIBRARIES})

install(TARGETS kritatiffexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_tiff.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

