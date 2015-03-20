if(LCMS2_FOUND)
  add_definitions( -DHAVE_LCMS2 )
endif()

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_tiff_test_SRCS kis_tiff_test.cpp )

add_executable(kis_tiff_testNAME krita-plugins-formats-tiff_test ${kis_tiff_test_SRCS})
add_test(kis_tiff_test kis_tiff_test)
ecm_mark_as_test(kis_tiff_test)

target_link_libraries(kis_tiff_test   kritaui Qt5::Test)
