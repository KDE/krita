set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/libs/widgets)

########### next target ###############

set(zoomhandler_test_SRCS zoomhandler_test.cpp )

kde4_add_unit_test(zoomhandler_test TESTNAME libs-widgets-zoomhandler_test  ${zoomhandler_test_SRCS})

target_link_libraries(zoomhandler_test kowidgets komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(zoomcontroller_test_SRCS zoomcontroller_test.cpp )

kde4_add_unit_test(zoomcontroller_test TESTNAME libs-widgets-zoomcontroller_test  ${zoomcontroller_test_SRCS})

target_link_libraries(zoomcontroller_test  kowidgets komain ${QT_QTTEST_LIBRARY})

########### end ###############
