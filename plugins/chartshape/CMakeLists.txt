add_subdirectory( tests )

include_directories( ${CMAKE_SOURCE_DIR}/plugins/chartshape
                     ${CMAKE_SOURCE_DIR}/plugins/chartshape/dialogs
                     ${CMAKE_SOURCE_DIR}/interfaces
                     ${KDCHART_INCLUDES}
                     ${KOMAIN_INCLUDES}
                     ${TEXTLAYOUT_INCLUDES}
)

########### Chart shape Plugin library ###############

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
    commands/LegendCommand.cpp
    commands/AxisCommand.cpp
    commands/DatasetCommand.cpp
    commands/ChartTextShapeCommand.cpp
)

kde4_add_ui_files(chartshape_LIB_SRCS
    ChartConfigWidget.ui
    dialogs/ChartTableEditor.ui
    dialogs/NewAxisDialog.ui
    dialogs/AxisScalingDialog.ui
    dialogs/CellRegionDialog.ui
    dialogs/FormatErrorBarDialog.ui
)

kde4_add_library(calligrachartshape SHARED ${chartshape_LIB_SRCS})
target_link_libraries(calligrachartshape ${KDCHART_LIBS} komain kotextlayout)
target_link_libraries(calligrachartshape LINK_INTERFACE_LIBRARIES ${KDCHART_LIBS})
set_target_properties(calligrachartshape PROPERTIES
    VERSION   ${GENERIC_CALLIGRA_LIB_VERSION}
    SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS calligrachartshape ${INSTALL_TARGETS_DEFAULT_ARGS})

kde4_add_plugin(calligra_shape_chart ChartShapeFactory.cpp )
target_link_libraries(calligra_shape_chart calligrachartshape ${KDCHART_LIBS} flake)
install(TARGETS calligra_shape_chart DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES chartshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
