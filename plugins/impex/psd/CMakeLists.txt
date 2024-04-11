add_subdirectory(tests)

set(LIB_PSD_SRCS 
    psd_colormode_block.cpp
    psd_image_data.cpp
)

kis_add_library( kritapsd_static STATIC ${LIB_PSD_SRCS})

target_link_libraries( kritapsd_static PUBLIC kritaimage kritapsd)

#
# import
#

set(kritapsdimport_SOURCES
    psd_import.cc
    psd_loader.cpp
)

kis_add_library(kritapsdimport MODULE ${kritapsdimport_SOURCES})

target_link_libraries(kritapsdimport kritapsd_static kritaglobal kritaui KF6::I18n)

install(TARGETS kritapsdimport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

#
# export
#

set(kritapsdexport_SOURCES
    psd_export.cc
    psd_saver.cpp
)

kis_add_library(kritapsdexport MODULE ${kritapsdexport_SOURCES})

target_link_libraries(kritapsdexport kritapsd_static kritaui kritapsd kritaimpex kritastore)

install(TARGETS kritapsdexport DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_psd.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
