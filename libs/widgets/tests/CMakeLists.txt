set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/libs/widgets)

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")

#add_subdirectory(filedialogtester)

########### next target ###############

set(zoomhandler_test_SRCS zoomhandler_test.cpp )
add_executable(zoomhandler_testNAME libs-widgets-zoomhandler_test  ${zoomhandler_test_SRCS})
add_test(zoomhandler_test zoomhandler_test)
ecm_mark_as_test(zoomhandler_test)
target_link_libraries(zoomhandler_test kowidgets Qt5::Test)

########### next target ###############

set(zoomcontroller_test_SRCS zoomcontroller_test.cpp )
add_executable(zoomcontroller_testNAME libs-widgets-zoomcontroller_test  ${zoomcontroller_test_SRCS})
add_test(zoomcontroller_test zoomcontroller_test)
ecm_mark_as_test(zoomcontroller_test)
target_link_libraries(zoomcontroller_test  kowidgets Qt5::Test)

########### next target ###############

set(KoResourceTaggingTest_SRCS KoResourceTaggingTest.cpp )
add_executable(KoResourceTaggingTestNAME libs-widgets-KoResourceTaggingTest  ${KoResourceTaggingTest_SRCS})
add_test(KoResourceTaggingTest KoResourceTaggingTest)
ecm_mark_as_test(KoResourceTaggingTest)
target_link_libraries(KoResourceTaggingTest  kowidgets Qt5::Test)

########### next target ###############

set(KoProgressUpdater_test_SRCS KoProgressUpdater_test.cpp )
add_executable(KoProgressUpdaterTestNAME libs-widgets-KoProgressUpdaterTest ${KoProgressUpdater_test_SRCS})
add_test(KoProgressUpdaterTest KoProgressUpdaterTest)
ecm_mark_as_test(KoProgressUpdaterTest)
target_link_libraries(KoProgressUpdaterTest kowidgets Qt5::Test KF5::ThreadWeaver)

########### end ###############
