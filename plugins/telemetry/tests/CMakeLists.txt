set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_SOURCE_DIR}/sdk/tests)

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_test(kis_telemetry_tests.cpp
    TEST_NAME krita-telemetryTest
    LINK_LIBRARIES kritaui  KUserFeedbackCore  kritaplugintelemetry  Qt5::Test)
add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")


