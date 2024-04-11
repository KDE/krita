add_subdirectory(tests)

set(libkritaconverter_LIB_SRCS
    jp2_converter.cc
)

set(kritajp2import_SOURCES
    jp2_import.cc
    ${libkritaconverter_LIB_SRCS}
)

kis_add_library(kritajp2import MODULE ${kritajp2import_SOURCES})

target_link_libraries(kritajp2import kritaui OpenJPEG::openjp2)

install(TARGETS kritajp2import DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_jp2.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
