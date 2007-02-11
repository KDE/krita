

if(KDE4_BUILD_TESTS)

include_directories( ${KOTEXT_INCLUDES} )

# all tests for obsolete classes
#   ########### kotextformattertest ###############
#
#   set(kotextformattertest_SRCS kotextformattertest.cpp )
#
#   kde4_automoc(${kotextformattertest_SRCS})
#
#   kde4_add_executable(kotextformattertest RUN_UNINSTALLED ${kotextformattertest_SRCS})
#
#   target_link_libraries(kotextformattertest kotext)
#
#   ########### kobordertest ###############
#
#   set(kobordertest_SRCS kobordertest.cpp )
#
#   kde4_automoc(${kobordertest_SRCS})
#
#   kde4_add_executable(kobordertest RUN_UNINSTALLED ${kobordertest_SRCS})
#
#   target_link_libraries(kobordertest kotext)
#
#   ########### kovariabletest ###############
#
#   set(kovariabletest_SRCS kovariabletest.cpp)
#
#   kde4_automoc(${kovariabletest_SRCS})
#
#   kde4_add_executable(kovariabletest RUN_UNINSTALLED ${kovariabletest_SRCS})
#
#   target_link_libraries(kovariabletest kotext ${QT_QTTEST_LIBRARY})
#
#   add_test(kotext-kovariabletest ${CMAKE_CURRENT_BINARY_DIR}/kovariabletest)
#
#   ########### KoUserStyleTester ###############
#
#   set(KoUserStyleTester_SRCS KoUserStyleTester.cpp)
#
#   kde4_automoc(${KoUserStyleTester_SRCS})
#
#   kde4_add_executable(KoUserStyleTester RUN_UNINSTALLED ${KoUserStyleTester_SRCS})
#
#   target_link_libraries(KoUserStyleTester kotext ${QT_QTTEST_LIBRARY})
#
#   add_test(kotext-KoUserStyleTester ${CMAKE_CURRENT_BINARY_DIR}/KoUserStyleTester)
#
#   ########### end ###############

########### next target ###############

set(TestDocumentLayout_test_SRCS TestDocumentLayout.cpp)
kde4_automoc(${TestDocumentLayout_test_SRCS})
kde4_add_executable(TestDocumentLayout RUN_UNINSTALLED ${TestDocumentLayout_test_SRCS})
target_link_libraries(TestDocumentLayout kotext ${QT_QTTEST_LIBRARY})
add_test(libs-kotext-TestDocumentLayout ${EXECUTABLE_OUTPUT_PATH}/TestDocumentLayout)

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_automoc(${TestBorder_test_SRCS})
kde4_add_executable(TestBorder RUN_UNINSTALLED ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kotext ${QT_QTTEST_LIBRARY})
add_test(libs-kotext-TestBorder ${EXECUTABLE_OUTPUT_PATH}/TestBorder)


endif(KDE4_BUILD_TESTS)
