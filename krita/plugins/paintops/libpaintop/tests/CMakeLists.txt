set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} ${CMAKE_SOURCE_DIR}/krita/image/metadata ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

set(kis_auto_brush_test_SRCS kis_auto_brush_test.cpp )
kde4_add_unit_test(KisAutobrushTest TESTNAME krita-image-KisAutobrushTest ${kis_auto_brush_test_SRCS})
target_link_libraries(KisAutobrushTest  ${KDE4_KDEUI_LIBS} komain kritaimage kritalibpaintop ${QT_QTTEST_LIBRARY})

set(kis_brush_test_SRCS kis_brush_test.cpp )
kde4_add_unit_test(KisBrushTest TESTNAME krita-image-KisBrushTest ${kis_brush_test_SRCS})
target_link_libraries(KisBrushTest  ${KDE4_KDEUI_LIBS} komain kritaimage kritalibpaintop ${QT_QTTEST_LIBRARY})

