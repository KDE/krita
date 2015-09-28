set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOWIDGETS_INCLUDES}  ${CMAKE_SOURCE_DIR}/libs/widgets)

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")

#add_subdirectory(filedialogtester)

########### next target ###############

set(zoomhandler_test_SRCS zoomhandler_test.cpp )
kde4_add_unit_test(zoomhandler_test TESTNAME libs-widgets-zoomhandler_test  ${zoomhandler_test_SRCS})
target_link_libraries(zoomhandler_test kowidgets Qt5::Test)

########### next target ###############

set(zoomcontroller_test_SRCS zoomcontroller_test.cpp )
kde4_add_unit_test(zoomcontroller_test TESTNAME libs-widgets-zoomcontroller_test  ${zoomcontroller_test_SRCS})
target_link_libraries(zoomcontroller_test  kowidgets Qt5::Test)

########### next target ###############

set(KoResourceTaggingTest_SRCS KoResourceTaggingTest.cpp )
kde4_add_unit_test(KoResourceTaggingTest TESTNAME libs-widgets-KoResourceTaggingTest  ${KoResourceTaggingTest_SRCS})
target_link_libraries(KoResourceTaggingTest  kowidgets Qt5::Test)

########### next target ###############

set(KoProgressUpdater_test_SRCS KoProgressUpdater_test.cpp )
kde4_add_unit_test(KoProgressUpdaterTest TESTNAME libs-widgets-KoProgressUpdaterTest ${KoProgressUpdater_test_SRCS})
target_link_libraries(KoProgressUpdaterTest kowidgets KF5::ThreadWeaver Qt5::Test)

########### end ###############
