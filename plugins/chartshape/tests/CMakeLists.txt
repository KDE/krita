set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_DIRECTORY} )

include_directories( ${CMAKE_SOURCE_DIR}/plugins/chartshape
                     #${CMAKE_SOURCE_DIR}/interfaces
                     #${CMAKE_SOURCE_DIR}/kchart/shape/dialogs
                     ${KOMAIN_INCLUDES} )

########### next target ###############
set( TestProxyModel_test_SRCS TestProxyModel.cpp )
set( TestDataSet_test_SRCS TestDataSet.cpp )
set( TestKChartModel_test_SRCS TestKChartModel.cpp ModelObserver.cpp )
set( TestTableSource_test_SRCS TestTableSource.cpp )
set( TestCellRegion_test_SRCS TestCellRegion.cpp )
kde4_add_unit_test( TestProxyModel TESTNAME kchart-TestProxyModel ${TestProxyModel_test_SRCS} )
kde4_add_unit_test( TestDataSet TESTNAME kchart-TestDataSet ${TestDataSet_test_SRCS} )
kde4_add_unit_test( TestKChartModel TESTNAME kchart-TestKChartModel ${TestKChartModel_test_SRCS} )
kde4_add_unit_test( TestTableSource TESTNAME kchart-TestTableSource ${TestTableSource_test_SRCS} )
kde4_add_unit_test( TestCellRegion TESTNAME kchart-TestCellRegion ${TestCellRegion_test_SRCS} )
#FIXME: Too many dependencies in DataSet, there should be no need to link to flake and the entire chart shape here
target_link_libraries( TestProxyModel chartshapecore Qt5::Test )
target_link_libraries( TestDataSet chartshapecore Qt5::Test )
target_link_libraries( TestKChartModel chartshapecore flake Qt5::Test )
target_link_libraries( TestTableSource chartshapecore Qt5::Test )
target_link_libraries( TestCellRegion chartshapecore Qt5::Test )

add_subdirectory( odf )

