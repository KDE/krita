include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

include(ECMAddTests)

if (APPLE)
    # cannot link to a MH_LIBRARY, see bug 417391
    krita_add_broken_unit_test(KisDabRenderingQueueTest.cpp
        TEST_NAME KisDabRenderingQueueTest
        LINK_LIBRARIES kritadefaultpaintops kritalibpaintop kritaimage Qt5::Test
        NAME_PREFIX "plugins-defaultpaintops-")
else ()
    ecm_add_test(KisDabRenderingQueueTest.cpp
        TEST_NAME KisDabRenderingQueueTest
        LINK_LIBRARIES kritadefaultpaintops kritalibpaintop kritaimage Qt5::Test
        NAME_PREFIX "plugins-defaultpaintops-")
endif()


krita_add_broken_unit_test(kis_brushop_test.cpp ../../../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME KisBrushOpTest
    LINK_LIBRARIES kritaui kritalibpaintop Qt5::Test
    NAME_PREFIX "plugins-defaultpaintops-")

