add_subdirectory(tests)

set(ICCJPEG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lcms")

include_directories(${ICCJPEG_SOURCE_DIR})

if(LCMS2_FOUND)
    include_directories( ${LCMS2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})
else()
    include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR} ${LCMS_INCLUDE_DIR})
endif()

set(libkritaconverter_LIB_SRCS
    kis_jpeg_converter.cc
    kis_jpeg_source.cpp
    kis_jpeg_destination.cpp
    ${ICCJPEG_SOURCE_DIR}/iccjpeg.c
    )

set(kritajpegimport_PART_SRCS
    kis_jpeg_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritajpegimport MODULE ${kritajpegimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritajpegimport krita_jpeg_import.desktop)

if(LCMS2_FOUND)
    target_link_libraries(kritajpegimport kritaui  ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} ${EXIV2_LIBRARIES} )
else()
    target_link_libraries(kritajpegimport kritaui  ${JPEG_LIBRARIES} ${LCMS_LIBRARIES} ${EXIV2_LIBRARIES} )
endif()

install(TARGETS kritajpegimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritajpegexport_PART_SRCS
    kis_jpeg_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

ki18n_wrap_ui(kritajpegexport_PART_SRCS kis_wdg_options_jpeg.ui )

add_library(kritajpegexport MODULE ${kritajpegexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritajpegexport krita_jpeg_export.desktop)

if(LCMS2_FOUND)
    target_link_libraries(kritajpegexport kritaui   ${JPEG_LIBRARIES} ${LCMS2_LIBRARIES} ${EXIV2_LIBRARIES} )
else()
    target_link_libraries(kritajpegexport kritaui   ${JPEG_LIBRARIES} ${LCMS_LIBRARIES} ${EXIV2_LIBRARIES} )
endif()

install(TARGETS kritajpegexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( PROGRAMS  krita_jpeg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
