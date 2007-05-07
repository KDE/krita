if(KDE4_BUILD_TESTS)

include_directories(
    ${KOTEXT_INCLUDES}
    ${QT_INCLUDES})

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
kde4_automoc(${TestStyles_test_SRCS})
kde4_add_executable(TestStyles RUN_UNINSTALLED ${TestStyles_test_SRCS})
target_link_libraries(TestStyles kotext ${QT_QTTEST_LIBRARY})
add_test(libs-kotext-styles-TestStyles ${EXECUTABLE_OUTPUT_PATH}/TestStyles)

########### next target ###############

set(TestListStyly_test_SRCS TestListStyle.cpp)
kde4_automoc(${TestListStyly_test_SRCS})
kde4_add_executable(TestListStyle RUN_UNINSTALLED ${TestListStyly_test_SRCS})
target_link_libraries(TestListStyle kotext ${QT_QTTEST_LIBRARY})
add_test(libs-kotext-styles-TestListStyle ${EXECUTABLE_OUTPUT_PATH}/TestListStyle)


endif(KDE4_BUILD_TESTS)
