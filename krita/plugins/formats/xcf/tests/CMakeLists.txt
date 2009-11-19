set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

########### next target ###############
set(kis_xcf_test_SRCS kis_xcf_test.cpp )

kde4_add_unit_test(kis_xcf_test TESTNAME krita-plugin-format-xcf_test ${kis_xcf_test_SRCS})

target_link_libraries(kis_xcf_test  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})
