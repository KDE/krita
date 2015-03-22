# To hide all the warnings from embedded 3rd party software like kdchart
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-switch)
endif ()


add_subdirectory( tests )


include_directories( ${CMAKE_SOURCE_DIR}/plugins/chartshape
                     ${CMAKE_SOURCE_DIR}/plugins/chartshape/dialogs
                     ${CMAKE_SOURCE_DIR}/interfaces
                     ${KDCHART_INCLUDES}
                     ${KOMAIN_INCLUDES}
                     ${TEXTLAYOUT_INCLUDES}
)

########### Chart shape Plugin library ###############

set (chartshape_LIB_SRCS
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

ki18n_wrap_ui(chartshape_LIB_SRCS
    ChartConfigWidget.ui
    dialogs/ChartTableEditor.ui
    dialogs/NewAxisDialog.ui
    dialogs/AxisScalingDialog.ui
    dialogs/CellRegionDialog.ui
    dialogs/FormatErrorBarDialog.ui
)

add_library(chartshapecore STATIC ${chartshape_LIB_SRCS})
target_link_libraries(chartshapecore  komain kotextlayout KChart)
if(NOT MSVC AND NOT (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "SunPro")
        set_target_properties(chartshapecore PROPERTIES COMPILE_FLAGS "-KPIC")
    else()
        set_target_properties(chartshapecore PROPERTIES COMPILE_FLAGS "-fPIC")
    endif()
endif()


add_library(calligra_shape_chart MODULE ChartShapeFactory.cpp )
target_link_libraries(calligra_shape_chart chartshapecore KChart flake)
install(TARGETS calligra_shape_chart DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_chart.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
