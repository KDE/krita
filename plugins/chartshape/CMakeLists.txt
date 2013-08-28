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

kde4_add_library(chartshapecore STATIC ${chartshape_LIB_SRCS})
target_link_libraries(chartshapecore ${KDCHART_LIBS} komain kotextlayout)
if(NOT MSVC AND NOT (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "SunPro")
        set_target_properties(chartshapecore PROPERTIES COMPILE_FLAGS "-KPIC")
    else("${CMAKE_CXX_COMPILER_ID}" MATCHES "SunPro")
        set_target_properties(chartshapecore PROPERTIES COMPILE_FLAGS "-fPIC")
    endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "SunPro")
endif(NOT MSVC AND NOT (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))


kde4_add_plugin(calligra_shape_chart ChartShapeFactory.cpp )
target_link_libraries(calligra_shape_chart chartshapecore ${KDCHART_LIBS} flake)
install(TARGETS calligra_shape_chart DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_chart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
