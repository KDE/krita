include_directories(
    ${KOTEXT_INCLUDES}
    ${QT_INCLUDES})

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
kde4_automoc(${TestStyles_test_SRCS})
kde4_add_unit_test(TestStyles TESTNAME libs-kotext-styles-TestStyles  ${TestStyles_test_SRCS})
target_link_libraries(TestStyles kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestListStyly_test_SRCS TestListStyle.cpp)
kde4_add_unit_test(TestListStyle TESTNAME libs-kotext-styles-TestListStyle  ${TestListStyly_test_SRCS})
target_link_libraries(TestListStyle kotext ${QT_QTTEST_LIBRARY})

