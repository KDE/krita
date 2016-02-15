########### next target ###############

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(    
    ${CMAKE_SOURCE_DIR}/libs/image/metadata 
    ${CMAKE_SOURCE_DIR}/sdk/tests 
    ${EIGEN3_INCLUDE_DIR}
)

if(HAVE_VC)
  include_directories(${Vc_INCLUDE_DIR})
endif()



macro_add_unittest_definitions()

########### next target ###############

set(kis_auto_brush_test_SRCS kis_auto_brush_test.cpp )
kde4_add_unit_test(KisAutobrushTest TESTNAME krita-libbrush-KisAutobrushTest ${kis_auto_brush_test_SRCS})
target_link_libraries(KisAutobrushTest   kritaimage kritalibbrush  Qt5::Test)

########### next target ###############

set(kis_brush_test_SRCS kis_brush_test.cpp )
kde4_add_unit_test(KisBrushTest TESTNAME krita-libbrush-KisBrushTest ${kis_brush_test_SRCS})
target_link_libraries(KisBrushTest   kritaimage kritalibbrush  Qt5::Test)

########### next target ###############

set(kis_boundary_test_SRCS kis_boundary_test.cpp )
kde4_add_unit_test(KisBoundaryTest TESTNAME krita-libbrush-KisBoundaryTest ${kis_boundary_test_SRCS})
target_link_libraries(KisBoundaryTest   kritaimage kritalibbrush  Qt5::Test)

########### next target ###############

set(kis_imagepipe_brush_test_SRCS kis_imagepipe_brush_test.cpp )
kde4_add_unit_test(KisImagePipeBrushTest TESTNAME krita-libbrush-KisImagePipeBrushTest ${kis_imagepipe_brush_test_SRCS})
target_link_libraries(KisImagePipeBrushTest   kritaimage kritalibbrush  Qt5::Test)
