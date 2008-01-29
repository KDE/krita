include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )

set(kis_tiled_data_tester_SRCS kis_tiled_data_tester.cpp )

kde4_add_unit_test(KisTiledDataTester TESTNAME krita-image-KisTiledDataTester  ${kis_tiled_data_tester_SRCS})

target_link_libraries(KisTiledDataTester  ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_tileswappertester_SRCS kis_tileswapper_tester.cpp )

kde4_add_unit_test(KisTileSwapperTester TESTNAME krita-image-KisTileSwapperTester  ${kis_tileswappertester_SRCS})

target_link_libraries(KisTileSwapperTester ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_tile_sharing_tester_SRCS kis_tile_sharing_tester.cpp )

kde4_add_unit_test(KisTileSharingTester TESTNAME krita-image-KisTileSharingTester  ${kis_tile_sharing_tester_SRCS})

target_link_libraries(KisTileSharingTester ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_tiles_regression_tester_SRCS kis_tiles_regression_tester.cpp )

kde4_add_unit_test(KisTilesRegressionTester TESTNAME krita-image-KisTilesRegressionTester  ${kis_tiles_regression_tester_SRCS})

target_link_libraries(KisTilesRegressionTester ${KDE4_KDEUI_LIBS} koguiutils komain kritaimage ${QT_QTTEST_LIBRARY})
