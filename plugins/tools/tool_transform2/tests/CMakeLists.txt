set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/sdk/tests
    ${Boost_INCLUDE_DIRS}
)

macro_add_unittest_definitions()

########### next target ###############

set(test_save_load_transform_args_SRCS test_save_load_transform_args.cpp )
kde4_add_broken_unit_test(TestSaveLoadTransformArgs TESTNAME krita-ui-TestSaveLoadTransformArgs  ${test_save_load_transform_args_SRCS})
target_link_libraries(TestSaveLoadTransformArgs    kritatooltransform kritaui kritaimage Qt5::Test)

set(test_animated_transform_parameters_SRCS test_animated_transform_parameters.cpp )
kde4_add_broken_unit_test(TestAnimatedTransformParameters TESTNAME krita-ui-TestAnimatedTransformParameters  ${test_animated_transform_parameters_SRCS})
target_link_libraries(TestAnimatedTransformParameters    kritatooltransform kritaui kritaimage Qt5::Test)
