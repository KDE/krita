set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_test(kis_watercolorop_test.cpp
    TEST_NAME krita-paintop-WetMapTest
    LINK_LIBRARIES kritaimage kritalibpaintop kritawatercolorpaintop Qt5::Test)
add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")

#krita_add_broken_unit_test(kis_watercolorop_test.cpp ../../../../sdk/tests/stroke_testing_utils.cpp
#    TEST_NAME krita-plugins-KisWaterColorOpTest
#    LINK_LIBRARIES kritaimage kritawatercolorpaintop kritalibpaintop Qt5::Test)
