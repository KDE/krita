add_subdirectory(tests)

if (TIFF_HAS_PSD_TAGS)
    set(libkritatiffpsd_SRCS
        kis_tiff_psd_layer_record.cpp
        kis_tiff_psd_resource_record.cpp
    )

    kis_add_library(kritatiffpsd SHARED ${libkritatiffpsd_SRCS})
    generate_export_header(kritatiffpsd)

    set_target_properties(kritatiffpsd PROPERTIES
        VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
    )

    target_link_libraries(kritatiffpsd kritapsd ${TIFF_LIBRARIES})

    install(TARGETS kritatiffpsd DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

    set(KRITATIFFPSD_LIBRARY kritatiffpsd)
endif()

set(kritatiffimport_SOURCES
    kis_tiff_import.cc
    kis_buffer_stream.cc
)

kis_add_library(kritatiffimport MODULE ${kritatiffimport_SOURCES})

target_link_libraries(kritatiffimport
    kritaui
    kritaimpex
    LibExiv2::LibExiv2
    kritametadata
    kritaexifcommon
    ${JPEG_TURBO_LIBRARIES}
    ${KRITATIFFPSD_LIBRARY}
    ${TIFF_LIBRARIES}
    )

install(TARGETS kritatiffimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritatiffexport_SOURCES
    kis_tiff_base_writer.cpp
    kis_tiff_converter.cc
    kis_tiff_writer_visitor.cpp
    kis_tiff_export.cc
    kis_dlg_options_tiff.cpp
)

if(TIFF_CAN_WRITE_PSD_TAGS)
set(kritatiffexport_SOURCES
    ${kritatiffexport_SOURCES}
    kis_tiff_psd_writer_visitor.cpp
)
endif()

ki18n_wrap_ui(kritatiffexport_SOURCES kis_wdg_options_tiff.ui)

kis_add_library(kritatiffexport MODULE ${kritatiffexport_SOURCES})

target_link_libraries(kritatiffexport
    kritaui
    kritaimpex
    LibExiv2::LibExiv2
    kritaexifcommon
    ${KRITATIFFPSD_LIBRARY}
    ${TIFF_LIBRARIES}
    )

install(TARGETS kritatiffexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install(FILES krita_tiff.desktop DESTINATION ${KDE_INSTALL_APPDIR})
