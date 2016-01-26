set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOWIDGETS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(test_save_load_transform_args_SRCS test_save_load_transform_args.cpp )
kde4_add_unit_test(TestSaveLoadTransformArgs TESTNAME krita-ui-TestSaveLoadTransformArgs  ${test_save_load_transform_args_SRCS})
target_link_libraries(TestSaveLoadTransformArgs    kritatooltransform kritaui kritaimage Qt5::Test)
