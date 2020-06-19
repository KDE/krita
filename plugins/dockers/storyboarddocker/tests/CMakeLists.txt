set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/.. 
    ${CMAKE_SOURCE_DIR}/libs/ui/tests 
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

include(ECMAddTests)

macro_add_unittest_definitions()

ecm_add_test(
    storyboardModelTest.cpp
    TEST_NAME kritastoryboardmodeltest
    NAME_PREFIX "plugins-dockers-storyboarddocker-"
    LINK_LIBRARIES kritastoryboarddocker kritaui Qt5::Test
)