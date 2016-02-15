add_subdirectory(tests)

set(ICCJPEG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lcms")

include_directories(${ICCJPEG_SOURCE_DIR})

include_directories( ${LCMS2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})

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

add_library(kritajpegimport MODULE ${kritajpegimport_SOURCES})

target_link_libraries(kritajpegimport kritaui  ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} ${EXIV2_LIBRARIES} )

install(TARGETS kritajpegimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritajpegexport_SOURCES
    kis_jpeg_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritajpegexport_SOURCES kis_wdg_options_jpeg.ui )

add_library(kritajpegexport MODULE ${kritajpegexport_SOURCES})

target_link_libraries(kritajpegexport kritaui   ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} ${EXIV2_LIBRARIES} )

install(TARGETS kritajpegexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_jpeg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
