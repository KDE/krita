

if(KDE4_BUILD_TESTS)

include_directories( ${KOTEXT_INCLUDES} )

########### kotextformattertest ###############

set(kotextformattertest_SRCS kotextformattertest.cpp )

kde4_automoc(${kotextformattertest_SRCS})

kde4_add_executable(kotextformattertest RUN_UNINSTALLED ${kotextformattertest_SRCS})

target_link_libraries(kotextformattertest kotext)

########### kobordertest ###############

set(kobordertest_SRCS kobordertest.cpp )

kde4_automoc(${kobordertest_SRCS})

kde4_add_executable(kobordertest RUN_UNINSTALLED ${kobordertest_SRCS})

target_link_libraries(kobordertest kotext)

########### kovariabletest ###############

set(kovariabletest_SRCS kovariabletest.cpp)

kde4_automoc(${kovariabletest_SRCS})

kde4_add_executable(kovariabletest RUN_UNINSTALLED ${kovariabletest_SRCS})

target_link_libraries(kovariabletest kotext ${QT_QTTEST_LIBRARY})

add_test(kotext-kovariabletest ${CMAKE_CURRENT_BINARY_DIR}/kovariabletest)

########### KoUserStyleTester ###############

set(KoUserStyleTester_SRCS KoUserStyleTester.cpp)

kde4_automoc(${KoUserStyleTester_SRCS})

kde4_add_executable(KoUserStyleTester RUN_UNINSTALLED ${KoUserStyleTester_SRCS})

target_link_libraries(KoUserStyleTester kotext ${QT_QTTEST_LIBRARY})

add_test(kotext-KoUserStyleTester ${CMAKE_CURRENT_BINARY_DIR}/KoUserStyleTester)

########### end ###############

endif(KDE4_BUILD_TESTS)
