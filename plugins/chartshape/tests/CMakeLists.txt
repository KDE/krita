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
kde4_add_unit_test( TestProxyModel TESTNAME kchart-TestProxyModel ${TestProxyModel_test_SRCS} )
kde4_add_unit_test( TestDataSet TESTNAME kchart-TestDataSet ${TestDataSet_test_SRCS} )
kde4_add_unit_test( TestKDChartModel TESTNAME kchart-TestKDChartModel ${TestKDChartModel_test_SRCS} )
kde4_add_unit_test( TestTableSource TESTNAME kchart-TestTableSource ${TestTableSource_test_SRCS} )
kde4_add_unit_test( TestCellRegion TESTNAME kchart-TestCellRegion ${TestCellRegion_test_SRCS} )
#FIXME: Too many dependencies in DataSet, there should be no need to link to flake and the entire chart shape here
target_link_libraries( TestProxyModel ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestDataSet ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestKDChartModel ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestTableSource ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestCellRegion ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshapelib)

add_subdirectory( odf )

