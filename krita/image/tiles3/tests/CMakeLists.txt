set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )
add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

########### next target ###############
set(kis_tiled_data_manager_test_SRCS kis_tiled_data_manager_test.cpp )
kde4_add_unit_test(KisTiledDataManagerTest TESTNAME krita-image-KisTiledDataManagerTest  ${kis_tiled_data_manager_test_SRCS})
target_link_libraries(KisTiledDataManagerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_low_memory_tests_SRCS kis_low_memory_tests.cpp )
kde4_add_unit_test(KisLowMemoryTests TESTNAME krita-image-KisLowMemoryTests  ${kis_low_memory_tests_SRCS})
target_link_libraries(KisLowMemoryTests  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_tile_compressors_test_SRCS kis_tile_compressors_test.cpp )
kde4_add_unit_test(KisTileCompressorsTest TESTNAME krita-image-KisTileCompressorsTest  ${kis_tile_compressors_test_SRCS})
target_link_libraries(KisTileCompressorsTest  ${KDE4_KDEUI_LIBS} koodf kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_compression_tests_SRCS kis_compression_tests.cpp )
kde4_add_unit_test(KisCompressionTests TESTNAME krita-image-KisCompressionTests  ${kis_compression_tests_SRCS})
target_link_libraries(KisCompressionTests  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_lockless_stack_test_SRCS kis_lockless_stack_test.cpp )
kde4_add_unit_test(KisLocklessStackTest TESTNAME krita-image-KisLocklessStackTest  ${kis_lockless_stack_test_SRCS})
target_link_libraries(KisLocklessStackTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_memory_pool_test_SRCS kis_memory_pool_test.cpp )
kde4_add_unit_test(KisMemoryPoolTest TESTNAME krita-image-KisMemoryPoolTest  ${kis_memory_pool_test_SRCS})
target_link_libraries(KisMemoryPoolTest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_chunk_allocator_test_SRCS kis_chunk_allocator_test.cpp ../swap/kis_chunk_allocator.cpp)
kde4_add_unit_test(KisChunkAllocatorTest TESTNAME krita-image-KisChunkAllocatorTest  ${kis_chunk_allocator_test_SRCS})
target_link_libraries(KisChunkAllocatorTest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_memory_window_test_SRCS kis_memory_window_test.cpp ../swap/kis_memory_window.cpp)
kde4_add_unit_test(KisMemoryWindowTest TESTNAME krita-image-KisMemoryWindowTest  ${kis_memory_window_test_SRCS})
target_link_libraries(KisMemoryWindowTest  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_swapped_data_store_test_SRCS kis_swapped_data_store_test.cpp ../kis_tile_data.cc)
kde4_add_unit_test(KisSwappedDataStoreTest TESTNAME krita-image-KisSwappedDataStoreTest  ${kis_swapped_data_store_test_SRCS})
target_link_libraries(KisSwappedDataStoreTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_tile_data_store_test_SRCS kis_tile_data_store_test.cpp ../kis_tile_data.cc)
kde4_add_unit_test(KisTileDataStoreTest TESTNAME krita-image-KisTileDataStoreTest  ${kis_tile_data_store_test_SRCS})
target_link_libraries(KisTileDataStoreTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_store_limits_test_SRCS kis_store_limits_test.cpp ../kis_tile_data.cc )
kde4_add_unit_test(KisStoreLimitsTest TESTNAME krita-image-KisStoreLimitsTest  ${kis_store_limits_test_SRCS})
target_link_libraries(KisStoreLimitsTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_tile_data_pooler_test_SRCS kis_tile_data_pooler_test.cpp ../kis_tile_data.cc ../kis_tile_data_pooler.cc )
kde4_add_unit_test(KisTileDataPoolerTest TESTNAME krita-image-KisTileDataPoolerTest  ${kis_tile_data_pooler_test_SRCS})
target_link_libraries(KisTileDataPoolerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

