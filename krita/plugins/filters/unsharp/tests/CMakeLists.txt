set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KRITA_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(kis_unsharp_mask_test_SRCS kis_unsharp_mask_test.cpp )
add_executable(KisUnsharpMaskTestNAME krita-ui-kis_unsharp_mask_test  ${kis_unsharp_mask_test_SRCS})
add_test(KisUnsharpMaskTest KisUnsharpMaskTest)
ecm_mark_as_test(KisUnsharpMaskTest)
target_link_libraries(KisUnsharpMaskTest   kritaui Qt5::Test)
