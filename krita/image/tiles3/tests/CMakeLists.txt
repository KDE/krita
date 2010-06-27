set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )
add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

########### next target ###############
set(kis_tiled_data_manager_test_SRCS kis_tiled_data_manager_test.cpp )
kde4_add_unit_test(KisTiledDataManagerTest TESTNAME krita-image-KisTiledDataManagerTest  ${kis_tiled_data_manager_test_SRCS})
target_link_libraries(KisTiledDataManagerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_legacy_tile_compressor_test_SRCS kis_legacy_tile_compressor_test.cpp )
kde4_add_unit_test(KisLegacyTileCompressorTest TESTNAME krita-image-KisLegacyTileCompressorTest  ${kis_legacy_tile_compressor_test_SRCS})
target_link_libraries(KisLegacyTileCompressorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_compression_tests_SRCS kis_compression_tests.cpp )
kde4_add_unit_test(KisCompressionTests TESTNAME krita-image-KisCompressionTests  ${kis_compression_tests_SRCS})
target_link_libraries(KisCompressionTests  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
