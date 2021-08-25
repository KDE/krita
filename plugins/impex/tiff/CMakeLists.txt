add_subdirectory(tests)

set(libkritatiffconverter_LIB_SRCS
    kis_tiff_converter.cc
    kis_tiff_writer_visitor.cpp
    kis_buffer_stream.cc
    kis_tiff_psd_layer_record.cpp
    kis_tiff_psd_resource_record.cpp
)

if(TIFF_HAS_PSD_TAGS)
    set(libkritatiffconverter_LIB_SRCS
        ${libkritatiffconverter_LIB_SRCS}
        kis_tiff_psd_layer_record.cpp
        kis_tiff_psd_resource_record.cpp
    )
endif()

if(TIFF_CAN_WRITE_PSD_TAGS)
    set(libkritatiffconverter_LIB_SRCS
        ${libkritatiffconverter_LIB_SRCS}
        kis_tiff_psd_writer_visitor.cpp
    )
endif()

set(kritatiffimport_SOURCES
    ${libkritatiffconverter_LIB_SRCS}
    kis_tiff_import.cc
    )


add_library(kritatiffimport MODULE ${kritatiffimport_SOURCES})

target_link_libraries(kritatiffimport kritaui kritapsd ${TIFF_LIBRARIES} LibExiv2::LibExiv2 kritametadata)

install(TARGETS kritatiffimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritatiffexport_SOURCES
    ${libkritatiffconverter_LIB_SRCS}
    kis_tiff_export.cc
    kis_dlg_options_tiff.cpp
    )

ki18n_wrap_ui(kritatiffexport_SOURCES kis_wdg_options_tiff.ui)

add_library(kritatiffexport MODULE ${kritatiffexport_SOURCES})

target_link_libraries(kritatiffexport kritaui kritaimpex kritapsd ${TIFF_LIBRARIES} LibExiv2::LibExiv2 kritametadata)

install(TARGETS kritatiffexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_tiff.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

