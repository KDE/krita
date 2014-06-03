add_subdirectory(tests)

if(LCMS2_FOUND)
    include_directories( ${LCMS2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR})
else(LCMS2_FOUND)
    include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata ${EXIV2_INCLUDE_DIR} ${LCMS_INCLUDE_DIR})
endif(LCMS2_FOUND)

set(libkritaconverter_LIB_SRCS
    kis_jpeg_converter.cc
    kis_jpeg_source.cpp
    kis_jpeg_destination.cpp
    iccjpeg.c
    )

set(kritajpegimport_PART_SRCS
    kis_jpeg_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritajpegimport ${kritajpegimport_PART_SRCS})

if(LCMS2_FOUND)
    target_link_libraries(kritajpegimport kritaui jpeg ${LCMS2_LIBRARIES} ${EXIV2_LIBRARIES} )
else(LCMS2_FOUND)
    target_link_libraries(kritajpegimport kritaui jpeg ${LCMS_LIBRARIES} ${EXIV2_LIBRARIES} )
endif(LCMS2_FOUND)

install(TARGETS kritajpegimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritajpegexport_PART_SRCS
    kis_jpeg_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_ui_files(kritajpegexport_PART_SRCS kis_wdg_options_jpeg.ui )

kde4_add_plugin(kritajpegexport ${kritajpegexport_PART_SRCS})

if(LCMS2_FOUND)
    target_link_libraries(kritajpegexport kritaui jpeg ${LCMS2_LIBRARIES} ${EXIV2_LIBRARIES} )
else(LCMS2_FOUND)
    target_link_libraries(kritajpegexport kritaui jpeg ${LCMS_LIBRARIES} ${EXIV2_LIBRARIES} )
endif(LCMS2_FOUND)

install(TARGETS kritajpegexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_jpeg_import.desktop krita_jpeg_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_jpeg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
