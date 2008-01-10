set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/plugins/tableshape ${KOGUIUTILS_INCLUDES} ${KOTEXT_INCLUDES} )

########### next target ###############

set(TestTableCell_test_SRCS
    TestTableCell.cpp
    ../TableCell.cpp
)
kde4_add_unit_test(TestTableCell TESTNAME shapes-table-TestTableCell ${TestTableCell_test_SRCS})
target_link_libraries(TestTableCell kotext ${QT_QTTEST_LIBRARY})
