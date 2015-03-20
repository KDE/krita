set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${KOTEXT_INCLUDES}
    )

add_definitions(-DSPECS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/specs/")

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
add_executable(TestStylesNAME libs-kotext-styles-TestStyles  ${TestStyles_test_SRCS})
add_test(TestStyles TestStyles)
ecm_mark_as_test(TestStyles)
target_link_libraries(TestStyles kotext Qt5::Test)

########### next target ###############

set(TestListStyly_test_SRCS TestListStyle.cpp)
add_executable(TestListStyleNAME libs-kotext-styles-TestListStyle  ${TestListStyly_test_SRCS})
add_test(TestListStyle TestListStyle)
ecm_mark_as_test(TestListStyle)
target_link_libraries(TestListStyle kotext Qt5::Test)

########### next target ###############

set(TestSectionStyle_test_SRCS TestSectionStyle.cpp)
add_executable(TestSectionStyleNAME libs-kotext-styles-TestSectionStyle ${TestSectionStyle_test_SRCS})
add_test(TestSectionStyle TestSectionStyle)
ecm_mark_as_test(TestSectionStyle)
target_link_libraries(TestSectionStyle kotext Qt5::Test)

########### next target ###############

set(TestTableCellStyle_test_SRCS TestTableCellStyle.cpp)
add_executable(TestTableCellStyleNAME libs-kotext-styles-TestTableCellStyle ${TestTableCellStyle_test_SRCS})
add_test(TestTableCellStyle TestTableCellStyle)
ecm_mark_as_test(TestTableCellStyle)
target_link_libraries(TestTableCellStyle kotext Qt5::Test)

########### next target ###############

set(TestStyleManager_test_SRCS TestStyleManager.cpp)
add_executable(TestStyleManagerNAME libs-kotext-styles-TestStyleManager ${TestStyleManager_test_SRCS})
add_test(TestStyleManager TestStyleManager)
ecm_mark_as_test(TestStyleManager)
target_link_libraries(TestStyleManager kotext Qt5::Test)

########### next target ###############

#set(TestOpenDocumentStyle_test_SRCS TestOpenDocumentStyle.cpp)
#add_executable(TestOpenDocumentStyleNAME libs-kotext-styles-TestOpenDocumentStyle ${TestOpenDocumentStyle_test_SRCS})
add_test(TestOpenDocumentStyle TestOpenDocumentStyle)
ecm_mark_as_test(TestOpenDocumentStyle)
#target_link_libraries(TestOpenDocumentStyle kotext Qt5::Test ${QT_XML_LIBRARY})
