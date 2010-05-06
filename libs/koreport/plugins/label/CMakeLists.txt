include_directories( ${KOMAIN_INCLUDES}
${KOODF_INCLUDES}
${CMAKE_SOURCE_DIR}/libs/
${CMAKE_SOURCE_DIR}/libs/koreport
${CMAKE_SOURCE_DIR}/libs/koreport/common
${CMAKE_SOURCE_DIR}/libs/koreport/renderer
${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
)

#build a shared library
set(koreport_labelplugin_LIB_SRCS
    KoReportItemLabel.cpp
    KoReportDesignerItemLabel.cpp
    krscriptlabel.cpp
    KoReportLabelPlugin.cpp
)

kde4_add_plugin(koreport_labelplugin ${koreport_labelplugin_LIB_SRCS})

target_link_libraries(koreport_labelplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} koreport ${KOPROPERTY_LIBS})

install(TARGETS koreport_labelplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  koreport_labelplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})