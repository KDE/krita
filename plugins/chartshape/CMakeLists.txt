add_subdirectory( kdchart )
add_subdirectory( tests )

include_directories( ${CMAKE_SOURCE_DIR}/plugins/chartshape
                     ${CMAKE_SOURCE_DIR}/plugins/chartshape/kdchart/include
                     ${CMAKE_SOURCE_DIR}/plugins/chartshape/dialogs
                     ${CMAKE_SOURCE_DIR}/interfaces
                     ${KOMAIN_INCLUDES}
                     ${TEXTLAYOUT_INCLUDES}
)

########### Flake Plugin library ###############

SET (chartshape_LIB_SRCS
    kchart_global.cpp
    KDChartConvertions.cpp
    Axis.cpp
    DataSet.cpp
    CellRegion.cpp
    CellRegionStringValidator.cpp
    ChartTableModel.cpp
    Legend.cpp
    TextLabelDummy.cpp
    PlotArea.cpp
    TableSource.cpp
    ChartProxyModel.cpp
    KDChartModel.cpp
    Surface.cpp
    ChartDocument.cpp
    ChartPart.cpp
    ChartShape.cpp
    ChartTool.cpp
    ChartToolFactory.cpp
    ChartConfigWidget.cpp
    ChartTableView.cpp
    ScreenConversions.cpp
    ChartLayout.cpp
    SingleModelHelper.cpp
    OdfLoadingHelper.cpp
    dialogs/TableEditorDialog.cpp
    dialogs/NewAxisDialog.cpp
    dialogs/AxisScalingDialog.cpp
    dialogs/CellRegionDialog.cpp
    dialogs/FontEditorDialog.cpp
    dialogs/FormatErrorBarDialog.cpp
    commands/ChartTypeCommand.cpp
)

kde4_add_ui_files(chartshape_LIB_SRCS
    ChartConfigWidget.ui
    dialogs/ChartTableEditor.ui
    dialogs/NewAxisDialog.ui
    dialogs/AxisScalingDialog.ui
    dialogs/CellRegionDialog.ui
    dialogs/FormatErrorBarDialog.ui
)

kde4_add_library(chartshapelib SHARED ${chartshape_LIB_SRCS})
target_link_libraries(chartshapelib kdchart komain textlayout)
target_link_libraries(chartshapelib LINK_INTERFACE_LIBRARIES kdchart)
set_target_properties(chartshapelib PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )

install(TARGETS chartshapelib ${INSTALL_TARGETS_DEFAULT_ARGS})

kde4_add_plugin(chartshape ChartShapeFactory.cpp )
target_link_libraries(chartshape chartshapelib kdchart flake)
install(TARGETS chartshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES chartshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kchartpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
