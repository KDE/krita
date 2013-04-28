if(LCMS2_FOUND)
  add_definitions( -DHAVE_LCMS2 )
endif(LCMS2_FOUND)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_tiff_test_SRCS kis_tiff_test.cpp )

kde4_add_unit_test(kis_tiff_test TESTNAME krita-plugins-formats-tiff_test ${kis_tiff_test_SRCS})

target_link_libraries(kis_tiff_test  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})
