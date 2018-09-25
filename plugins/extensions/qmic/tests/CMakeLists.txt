set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_BINARY_DIR}/plugins/extensions/qmic
)

macro_add_unittest_definitions()

ecm_add_test(kis_qmic_tests.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../kis_qmic_simple_convertor.cpp
    TEST_NAME kis_qmic_tests
    LINK_LIBRARIES kritaimage Qt5::Test
    NAME_PREFIX "plugins-extensions-qmic-")
