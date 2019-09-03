add_subdirectory(tests)

set(ICCJPEG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lcms")

include_directories(
    ${ICCJPEG_SOURCE_DIR}
)

include_directories(SYSTEM
    ${LCMS2_INCLUDE_DIR}
)

set(libkritaconverter_LIB_SRCS
    kis_jpeg_converter.cc
    kis_jpeg_source.cpp
    kis_jpeg_destination.cpp
    ${ICCJPEG_SOURCE_DIR}/iccjpeg.c
    )

set(kritajpegimport_SOURCES
    kis_jpeg_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kis_add_library(kritajpegimport MODULE ${kritajpegimport_SOURCES})

target_link_libraries(kritajpegimport kritaui  ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} LibExiv2::LibExiv2 )

install(TARGETS kritajpegimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritajpegexport_SOURCES
    kis_jpeg_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritajpegexport_SOURCES kis_wdg_options_jpeg.ui )

kis_add_library(kritajpegexport MODULE ${kritajpegexport_SOURCES})

target_link_libraries(kritajpegexport kritaui kritaimpex ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} LibExiv2::LibExiv2 )

install(TARGETS kritajpegexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_jpeg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
