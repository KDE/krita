set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_exr_test_SRCS kis_exr_test.cpp )

kde4_add_unit_test(kis_exr_test TESTNAME krita-fileformat-kis_exr_test ${kis_exr_test_SRCS})

target_link_libraries(kis_exr_test  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})
