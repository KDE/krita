set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

set(kis_tiled_data_tester_SRCS kis_tiled_data_tester.cpp )

kde4_add_unit_test(KisTiledDataTester TESTNAME krita-image-KisTiledDataTester  ${kis_tiled_data_tester_SRCS})

target_link_libraries(KisTiledDataTester  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
