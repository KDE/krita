include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

set(kis_tiled_data_tester_SRCS kis_tiled_data_tester.cpp )

kde4_automoc(${kis_tiled_data_tester_SRCS})

kde4_add_unit_test(KisTiledDataTester TESTNAME krita-image-KisTiledDataTester  ${kis_tiled_data_tester_SRCS})

target_link_libraries(KisTiledDataTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
