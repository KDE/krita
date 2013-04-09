include_directories(
    ${KOPROPERTY_INCLUDES}
    ${KOREPORT_INCLUDES}
    ${KDCHART_INCLUDES}
)

#build a shared library
set(koreport_chartplugin_LIB_SRCS
    KoReportItemChart.cpp
    KoReportDesignerItemChart.cpp
    krscriptchart.cpp
    KoReportChartPlugin.cpp
)

kde4_add_plugin(koreport_chartplugin ${koreport_chartplugin_LIB_SRCS})

target_link_libraries(koreport_chartplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KOREPORT_LIBS} kdchart ${KOPROPERTY_LIBS})

install(TARGETS koreport_chartplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  koreport_chartplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
