include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

include(ECMAddTests)

ecm_add_test(
    KisColorsmudgeOpTest.cpp
    ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME KisColorsmudgeOpTest
    LINK_LIBRARIES kritalibpaintop kritaimage Qt5::Test
    NAME_PREFIX "plugins-colorsmudge-")
