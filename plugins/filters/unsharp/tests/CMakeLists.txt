set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

set(kis_unsharp_mask_test_SRCS kis_unsharp_mask_test.cpp )
kde4_add_unit_test(KisUnsharpMaskTest TESTNAME krita-ui-kis_unsharp_mask_test  ${kis_unsharp_mask_test_SRCS})
target_link_libraries(KisUnsharpMaskTest   kritaui Qt5::Test)
