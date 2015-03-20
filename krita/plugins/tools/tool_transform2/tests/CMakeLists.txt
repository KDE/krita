set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(test_save_load_transform_args_SRCS test_save_load_transform_args.cpp )
add_executable(TestSaveLoadTransformArgsNAME krita-ui-TestSaveLoadTransformArgs  ${test_save_load_transform_args_SRCS})
add_test(TestSaveLoadTransformArgs TestSaveLoadTransformArgs)
ecm_mark_as_test(TestSaveLoadTransformArgs)
target_link_libraries(TestSaveLoadTransformArgs    kritatooltransform kritaui kritaimage Qt5::Test)
