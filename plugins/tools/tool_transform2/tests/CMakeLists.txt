set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/sdk/tests
    ${Boost_INCLUDE_DIRS}
)

macro_add_unittest_definitions()

########### next target ###############

add_test(test_save_load_transform_args.cpp
    TEST_NAME krita-ui-TestSaveLoadTransformArgs
    LINK_LIBRARIES kritatooltransform kritaui kritaimage Qt5::Test)

add_test(test_animated_transform_parameters.cpp
    TEST_NAME krita-ui-TestAnimatedTransformParameters
    LINK_LIBRARIES kritatooltransform kritaui kritaimage Qt5::Test)
