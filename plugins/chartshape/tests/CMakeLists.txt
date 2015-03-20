set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_DIRECTORY} )

include_directories( ${CMAKE_SOURCE_DIR}/plugins/chartshape
                     #${CMAKE_SOURCE_DIR}/interfaces
                     #${CMAKE_SOURCE_DIR}/kchart/shape/dialogs
                     ${KDCHART_INCLUDES}
                     ${KOMAIN_INCLUDES} )

########### next target ###############
set( TestProxyModel_test_SRCS TestProxyModel.cpp )
set( TestDataSet_test_SRCS TestDataSet.cpp )
set( TestKDChartModel_test_SRCS TestKDChartModel.cpp ModelObserver.cpp )
set( TestTableSource_test_SRCS TestTableSource.cpp )
set( TestCellRegion_test_SRCS TestCellRegion.cpp )
add_executable( TestProxyModelNAME kchart-TestProxyModel ${TestProxyModel_test_SRCS} )
add_test(TestProxyModel TestProxyModel)
ecm_mark_as_test(TestProxyModel)
add_executable( TestDataSetNAME kchart-TestDataSet ${TestDataSet_test_SRCS} )
add_test(TestDataSet TestDataSet)
ecm_mark_as_test(TestDataSet)
add_executable( TestKDChartModelNAME kchart-TestKDChartModel ${TestKDChartModel_test_SRCS} )
add_test(TestKDChartModel TestKDChartModel)
ecm_mark_as_test(TestKDChartModel)
add_executable( TestTableSourceNAME kchart-TestTableSource ${TestTableSource_test_SRCS} )
add_test(TestTableSource TestTableSource)
ecm_mark_as_test(TestTableSource)
add_executable( TestCellRegionNAME kchart-TestCellRegion ${TestCellRegion_test_SRCS} )
add_test(TestCellRegion TestCellRegion)
ecm_mark_as_test(TestCellRegion)
#FIXME: Too many dependencies in DataSet, there should be no need to link to flake and the entire chart shape here
target_link_libraries( TestProxyModel Qt5::Core Qt5::Gui Qt5::Test chartshapecore )
target_link_libraries( TestDataSet Qt5::Core Qt5::Gui Qt5::Test chartshapecore )
target_link_libraries( TestKDChartModel  flake Qt5::Test chartshapecore )
target_link_libraries( TestTableSource Qt5::Core Qt5::Gui Qt5::Test chartshapecore )
target_link_libraries( TestCellRegion Qt5::Core Qt5::Gui Qt5::Test chartshapecore)

add_subdirectory( odf )

