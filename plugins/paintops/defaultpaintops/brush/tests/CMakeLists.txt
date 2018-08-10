set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

include(ECMAddTests)

ecm_add_test(KisDabRenderingQueueTest.cpp
    TEST_NAME plugins-defaultpaintops-KisDabRenderingQueueTest
    LINK_LIBRARIES kritadefaultpaintops kritalibpaintop kritaimage Qt5::Test)



krita_add_broken_unit_test(kis_brushop_test.cpp ../../../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME plugins-defaultpaintops-KisBrushOpTest
    LINK_LIBRARIES kritaimage kritaui kritalibpaintop Qt5::Test)

