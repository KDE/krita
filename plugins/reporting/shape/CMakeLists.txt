include_directories( ${KOMAIN_INCLUDES}
${KOODF_INCLUDES}
${KPLATO_INCLUDES}
${CMAKE_SOURCE_DIR}/libs/
${CMAKE_SOURCE_DIR}/libs/koreport
${CMAKE_SOURCE_DIR}/libs/koreport/common
${CMAKE_SOURCE_DIR}/libs/koreport/renderer
${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
)

#build a shared library
set(koreport_shapeplugin_LIB_SRCS
    KoReportItemShape.cpp
    KoReportDesignerItemShape.cpp
#    krscriptshape.cpp
    KoReportShapePlugin.cpp
)

kde4_add_plugin(koreport_shapeplugin ${koreport_shapeplugin_LIB_SRCS})

target_link_libraries(koreport_shapeplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} koreport komain ${KOPROPERTY_LIBS})


########### install files ###############

install(TARGETS koreport_shapeplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  koreport_shapeplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})