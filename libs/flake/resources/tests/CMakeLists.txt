set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)

include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

# needs kritaimage for TestUtil
ecm_add_tests(
    KoGamutMaskTest.cpp
#    KoGamutMaskShapeTest.cpp
    LINK_LIBRARIES kritaflake kritaimage Qt5::Test
    NAME_PREFIX "libs-flake-")
