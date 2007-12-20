
include_directories( ${KRITA_INCLUDES} )

set(libkritaconverter_LIB_SRCS
    %{APPNAMELC}_converter.cc
    )

set(krita%{APPNAMELC}import_PART_SRCS
    %{APPNAMELC}_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(krita%{APPNAMELC}import WITH_PREFIX ${krita%{APPNAMELC}import_PART_SRCS})



target_link_libraries(krita%{APPNAMELC}import kritaui )

install(TARGETS krita%{APPNAMELC}import  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(krita%{APPNAMELC}export_PART_SRCS
    %{APPNAMELC}_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(krita%{APPNAMELC}export WITH_PREFIX ${krita%{APPNAMELC}export_PART_SRCS})


target_link_libraries(krita%{APPNAMELC}export kritaui )

install(TARGETS krita%{APPNAMELC}export  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_%{APPNAMELC}_import.desktop krita_%{APPNAMELC}_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_%{APPNAMELC}.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
