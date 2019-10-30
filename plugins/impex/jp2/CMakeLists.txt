add_subdirectory(tests)

include_directories(SYSTEM ${OpenJPEG_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    jp2_converter.cc
)

set(kritajp2import_SOURCES
    jp2_import.cc
    ${libkritaconverter_LIB_SRCS}
)

add_library(kritajp2import MODULE ${kritajp2import_SOURCES})

target_link_libraries(kritajp2import kritaui ${OpenJPEG_LIBRARIES} )

install(TARGETS kritajp2import DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

#set(kritajp2export_SOURCES
#    jp2_export.cc
#    ${libkritaconverter_LIB_SRCS}
#)
#
#ki18n_wrap_ui(kritajp2export_SOURCES kis_wdg_options_jp2.ui )
#
#add_library(kritajp2export MODULE ${kritajp2export_SOURCES})
#
#target_link_libraries(kritajp2export kritaui  ${OPENJPEG_LIBRARIES} )
#install(TARGETS kritajp2export  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_jp2.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
