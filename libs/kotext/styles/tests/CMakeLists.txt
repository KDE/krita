set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${KOTEXT_INCLUDES}
    ${QT_INCLUDES})

add_definitions(-DSPECS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/specs/")

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
kde4_add_unit_test(TestStyles TESTNAME libs-kotext-styles-TestStyles  ${TestStyles_test_SRCS})
target_link_libraries(TestStyles kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestListStyly_test_SRCS TestListStyle.cpp)
kde4_add_unit_test(TestListStyle TESTNAME libs-kotext-styles-TestListStyle  ${TestListStyly_test_SRCS})
target_link_libraries(TestListStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSectionStyle_test_SRCS TestSectionStyle.cpp)
kde4_add_unit_test(TestSectionStyle TESTNAME libs-kotext-styles-TestSectionStyle ${TestSectionStyle_test_SRCS})
target_link_libraries(TestSectionStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTableCellStyle_test_SRCS TestTableCellStyle.cpp)
kde4_add_unit_test(TestTableCellStyle TESTNAME libs-kotext-styles-TestTableCellStyle ${TestTableCellStyle_test_SRCS})
target_link_libraries(TestTableCellStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOpenDocumentStyle_test_SRCS TestOpenDocumentStyle.cpp)
kde4_add_unit_test(TestOpenDocumentStyle TESTNAME libs-kotext-styles-TestOpenDocumentStyle ${TestOpenDocumentStyle_test_SRCS})
target_link_libraries(TestOpenDocumentStyle kotext ${QT_QTTEST_LIBRARY} ${QT_XML_LIBRARY})
