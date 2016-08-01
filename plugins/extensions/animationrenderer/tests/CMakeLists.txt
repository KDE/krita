set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_video_plugin_test_SRCS kis_video_plugin_test.cpp )

kde4_add_unit_test(kis_video_plugin_test TESTNAME krita-plugins-formats-video_test ${kis_video_plugin_test_SRCS})

target_link_libraries(kis_video_plugin_test kritavideoexport kritaui Qt5::Test)
