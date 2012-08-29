########### next target ###############

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/krita/image/metadata ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

########### next target ###############

set(kis_auto_brush_test_SRCS kis_auto_brush_test.cpp )
kde4_add_unit_test(KisAutobrushTest TESTNAME krita-libbrush-KisAutobrushTest ${kis_auto_brush_test_SRCS})
target_link_libraries(KisAutobrushTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibbrush ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_brush_test_SRCS kis_brush_test.cpp )
kde4_add_unit_test(KisBrushTest TESTNAME krita-libbrush-KisBrushTest ${kis_brush_test_SRCS})
target_link_libraries(KisBrushTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibbrush ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_boundary_test_SRCS kis_boundary_test.cpp )
kde4_add_unit_test(KisBoundaryTest TESTNAME krita-libbrush-KisBoundaryTest ${kis_boundary_test_SRCS})
target_link_libraries(KisBoundaryTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibbrush ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_imagepipe_brush_test_SRCS kis_imagepipe_brush_test.cpp )
kde4_add_unit_test(KisImagePipeBrushTest TESTNAME krita-libbrush-KisImagePipeBrushTest ${kis_imagepipe_brush_test_SRCS})
target_link_libraries(KisImagePipeBrushTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibbrush ${QT_QTTEST_LIBRARY})
