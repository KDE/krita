########### next target ###############

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(    ${CMAKE_SOURCE_DIR}/krita/image/metadata ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(kis_auto_brush_test_SRCS kis_auto_brush_test.cpp )
add_executable(KisAutobrushTestNAME krita-libbrush-KisAutobrushTest ${kis_auto_brush_test_SRCS})
add_test(KisAutobrushTest KisAutobrushTest)
ecm_mark_as_test(KisAutobrushTest)
target_link_libraries(KisAutobrushTest   kritaimage kritalibbrush Qt5::Test)

########### next target ###############

set(kis_brush_test_SRCS kis_brush_test.cpp )
add_executable(KisBrushTestNAME krita-libbrush-KisBrushTest ${kis_brush_test_SRCS})
add_test(KisBrushTest KisBrushTest)
ecm_mark_as_test(KisBrushTest)
target_link_libraries(KisBrushTest   kritaimage kritalibbrush Qt5::Test)

########### next target ###############

set(kis_boundary_test_SRCS kis_boundary_test.cpp )
add_executable(KisBoundaryTestNAME krita-libbrush-KisBoundaryTest ${kis_boundary_test_SRCS})
add_test(KisBoundaryTest KisBoundaryTest)
ecm_mark_as_test(KisBoundaryTest)
target_link_libraries(KisBoundaryTest   kritaimage kritalibbrush Qt5::Test)

########### next target ###############

set(kis_imagepipe_brush_test_SRCS kis_imagepipe_brush_test.cpp )
add_executable(KisImagePipeBrushTestNAME krita-libbrush-KisImagePipeBrushTest ${kis_imagepipe_brush_test_SRCS})
add_test(KisImagePipeBrushTest KisImagePipeBrushTest)
ecm_mark_as_test(KisImagePipeBrushTest)
target_link_libraries(KisImagePipeBrushTest   kritaimage kritalibbrush Qt5::Test)
