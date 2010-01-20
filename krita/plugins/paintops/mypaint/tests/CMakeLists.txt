set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")

########### next target ###############

set(kis_mypaint_surface_test_SRCS kis_mypaint_surface_test.cpp ../mypaint_surface.cpp)
kde4_add_unit_test(KisMyPaintSurfaceTest TESTNAME krita-mypaint-KisMyPaintSurfaceTest ${kis_mypaint_surface_test_SRCS})
target_link_libraries(KisMyPaintSurfaceTest  kritaimage ${QT_QTTEST_LIBRARY})

