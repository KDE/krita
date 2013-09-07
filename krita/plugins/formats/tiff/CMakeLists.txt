add_subdirectory(tests)

set(libkritatiffconverter_LIB_SRCS
    kis_tiff_converter.cc
    kis_tiff_writer_visitor.cpp
    kis_tiff_reader.cc
    kis_tiff_ycbcr_reader.cc
    kis_buffer_stream.cc
    )

########## next target ###############

set(kritatiffimport_PART_SRCS
    ${libkritatiffconverter_LIB_SRCS}
    kis_tiff_import.cc
    )


kde4_add_plugin(kritatiffimport ${kritatiffimport_PART_SRCS})



target_link_libraries(kritatiffimport kritaui ${TIFF_LIBRARIES})

install(TARGETS kritatiffimport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kritatiffexport_PART_SRCS
    ${libkritatiffconverter_LIB_SRCS}
    kis_tiff_export.cc
    kis_dlg_options_tiff.cpp
    )

kde4_add_ui_files(kritatiffexport_PART_SRCS kis_wdg_options_tiff.ui )

kde4_add_plugin(kritatiffexport ${kritatiffexport_PART_SRCS})



target_link_libraries(kritatiffexport kritaui ${TIFF_LIBRARIES})

install(TARGETS kritatiffexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES  krita_tiff_export.desktop krita_tiff_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_tiff.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

