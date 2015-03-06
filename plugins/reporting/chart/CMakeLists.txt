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

add_library(koreport_chartplugin MODULE ${koreport_chartplugin_LIB_SRCS})

target_link_libraries(koreport_chartplugin   ${KOREPORT_LIBS} ${KDCHART_LIBS} ${KOPROPERTY_LIBS})

install(TARGETS koreport_chartplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  koreport_chartplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
