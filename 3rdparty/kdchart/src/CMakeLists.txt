

## KDE4 includes needed for kdemacros.h
include_directories(
        ${KDE4_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}/../include 
	${CMAKE_CURRENT_SOURCE_DIR}/../kdablibfakes/include 
)

ADD_DEFINITIONS(-DKDCHART_BUILD_KDCHART_LIB)

########### next target ###############

set(kdchart_LIB_SRCS 
    KDChartAbstractArea.cpp
    KDChartAbstractAreaBase.cpp
    KDChartAbstractAreaWidget.cpp
    KDChartAbstractAxis.cpp
    KDChartAbstractCartesianDiagram.cpp
    KDChartAbstractCoordinatePlane.cpp
    KDChartAbstractDiagram.cpp
    KDChartAbstractGrid.cpp
    KDChartAbstractPieDiagram.cpp
    KDChartAbstractPolarDiagram.cpp
    KDChartAbstractProxyModel.cpp
    KDChartAbstractThreeDAttributes.cpp
    KDChartAttributesModel.cpp
    KDChartBackgroundAttributes.cpp
    KDChartBarAttributes.cpp
    KDChartBarDiagram.cpp
    KDChartBarDiagram_p.cpp
    KDChartCartesianAxis.cpp
    KDChartCartesianCoordinatePlane.cpp
    KDChartCartesianDiagramDataCompressor_p.cpp
    KDChartCartesianGrid.cpp
    KDChartChart.cpp
    KDChartDatasetProxyModel.cpp
    KDChartDatasetSelector.cpp
    KDChartDataValueAttributes.cpp
    KDChartDiagramObserver.cpp
    KDChartFrameAttributes.cpp
    KDChartGridAttributes.cpp
    KDChartHeaderFooter.cpp
    KDChartLayoutItems.cpp
    KDChartLegend.cpp
    KDChartLineAttributes.cpp
    KDChartLineDiagram.cpp
    KDChartLineDiagram_p.cpp
    KDChartMarkerAttributes.cpp
    KDChartMeasure.cpp
    KDChartModelDataCache_p.cpp
    KDChartNormalBarDiagram_p.cpp
    KDChartNormalLineDiagram_p.cpp
    KDChartNormalLyingBarDiagram_p.cpp
    KDChartNormalPlotter_p.cpp
    KDChartPaintContext.cpp
    KDChartPalette.cpp
    KDChartPercentBarDiagram_p.cpp
    KDChartPercentLineDiagram_p.cpp
    KDChartPercentLyingBarDiagram_p.cpp
    KDChartPercentPlotter_p.cpp
    KDChartPieAttributes.cpp
    KDChartPieDiagram.cpp
    KDChartPlotter.cpp
    KDChartPlotter_p.cpp
    KDChartPolarCoordinatePlane.cpp
    KDChartPolarDiagram.cpp
    KDChartPolarGrid.cpp
    KDChartRadarDiagram.cpp
    KDChartRadarCoordinatePlane.cpp
    KDChartRadarGrid.cpp
    KDChartPosition.cpp
    KDChartPrintingParameters.cpp
    KDChartRelativePosition.cpp
    KDChartRingDiagram.cpp
    KDChartRulerAttributes.cpp
    KDChartSignalCompressor.cpp
    KDChartStackedBarDiagram_p.cpp
    KDChartStackedLineDiagram_p.cpp
    KDChartStackedLyingBarDiagram_p.cpp
    KDChartStockBarAttributes.cpp
    KDChartStockDiagram.cpp
    KDChartStockDiagram_p.cpp
    KDChartTextArea.cpp
    KDChartTextAttributes.cpp
    KDChartThreeDBarAttributes.cpp
    KDChartThreeDLineAttributes.cpp
    KDChartThreeDPieAttributes.cpp
    KDChartValueTrackerAttributes.cpp
    KDChartWidget.cpp
    KDTextDocument.cpp
    LeveyJennings/KDChartLeveyJenningsAxis.cpp
    LeveyJennings/KDChartLeveyJenningsCoordinatePlane.cpp
    LeveyJennings/KDChartLeveyJenningsDiagram.cpp
    LeveyJennings/KDChartLeveyJenningsDiagram_p.cpp
    LeveyJennings/KDChartLeveyJenningsGrid.cpp
    LeveyJennings/KDChartLeveyJenningsGridAttributes.cpp
    PrerenderedElements/KDChartTextLabelCache.cpp
    Scenery/ChartGraphicsItem.cpp
    Scenery/ReverseMapper.cpp
    Ternary/KDChartAbstractTernaryDiagram.cpp
    Ternary/KDChartTernaryAxis.cpp
    Ternary/KDChartTernaryCoordinatePlane.cpp
    Ternary/KDChartTernaryGrid.cpp
    Ternary/KDChartTernaryLineDiagram.cpp
    Ternary/KDChartTernaryPointDiagram.cpp
    Ternary/TernaryConstants.cpp
    Ternary/TernaryPoint.cpp
)

qt4_wrap_ui(kdchart_LIB_SRCS
    KDChartDatasetSelector.ui
)

qt_wrap_cpp(kdchart kdchart_LIB_SRCS KDChartEnums.h)

kde4_add_library(kdchart SHARED ${kdchart_LIB_SRCS})

target_link_libraries(kdchart  ${QT_QTSVG_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARY} )

set_target_properties(kdchart PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS kdchart  ${INSTALL_TARGETS_DEFAULT_ARGS})
#add_subdirectory( kdchart )


########### install files ###############


#NOTHING
