set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   ${KOGUIUTILS_INCLUDES} )

macro_add_unittest_definitions()

########### next target ###############
set(kis_tiled_data_manager_test_SRCS kis_tiled_data_manager_test.cpp )
add_executable(KisTiledDataManagerTestNAME krita-image-KisTiledDataManagerTest  ${kis_tiled_data_manager_test_SRCS})
add_test(KisTiledDataManagerTest KisTiledDataManagerTest)
ecm_mark_as_test(KisTiledDataManagerTest)
target_link_libraries(KisTiledDataManagerTest   kritaimage Qt5::Test)

########### next target ###############
set(kis_low_memory_tests_SRCS kis_low_memory_tests.cpp )
add_executable(KisLowMemoryTestsNAME krita-image-KisLowMemoryTests  ${kis_low_memory_tests_SRCS})
add_test(KisLowMemoryTests KisLowMemoryTests)
ecm_mark_as_test(KisLowMemoryTests)
target_link_libraries(KisLowMemoryTests   kritaimage Qt5::Test)
set_tests_properties(krita-image-KisLowMemoryTests PROPERTIES TIMEOUT 180)

########### next target ###############
set(kis_tile_compressors_test_SRCS kis_tile_compressors_test.cpp )
add_executable(KisTileCompressorsTestNAME krita-image-KisTileCompressorsTest  ${kis_tile_compressors_test_SRCS})
add_test(KisTileCompressorsTest KisTileCompressorsTest)
ecm_mark_as_test(KisTileCompressorsTest)
target_link_libraries(KisTileCompressorsTest   koodf kritaimage Qt5::Test)

########### next target ###############
set(kis_compression_tests_SRCS kis_compression_tests.cpp )
add_executable(KisCompressionTestsNAME krita-image-KisCompressionTests  ${kis_compression_tests_SRCS})
add_test(KisCompressionTests KisCompressionTests)
ecm_mark_as_test(KisCompressionTests)
target_link_libraries(KisCompressionTests   kritaimage Qt5::Test)

########### next target ###############
set(kis_lockless_stack_test_SRCS kis_lockless_stack_test.cpp )
add_executable(KisLocklessStackTestNAME krita-image-KisLocklessStackTest  ${kis_lockless_stack_test_SRCS})
add_test(KisLocklessStackTest KisLocklessStackTest)
ecm_mark_as_test(KisLocklessStackTest)
target_link_libraries(KisLocklessStackTest   kritaimage Qt5::Test)

########### next target ###############
set(kis_memory_pool_test_SRCS kis_memory_pool_test.cpp )
add_executable(KisMemoryPoolTestNAME krita-image-KisMemoryPoolTest  ${kis_memory_pool_test_SRCS})
add_test(KisMemoryPoolTest KisMemoryPoolTest)
ecm_mark_as_test(KisMemoryPoolTest)
target_link_libraries(KisMemoryPoolTest   Qt5::Test)

########### next target ###############
set(kis_chunk_allocator_test_SRCS kis_chunk_allocator_test.cpp ../swap/kis_chunk_allocator.cpp)
add_executable(KisChunkAllocatorTestNAME krita-image-KisChunkAllocatorTest  ${kis_chunk_allocator_test_SRCS})
add_test(KisChunkAllocatorTest KisChunkAllocatorTest)
ecm_mark_as_test(KisChunkAllocatorTest)
target_link_libraries(KisChunkAllocatorTest   Qt5::Test)

########### next target ###############
set(kis_memory_window_test_SRCS kis_memory_window_test.cpp ../swap/kis_memory_window.cpp)
add_executable(KisMemoryWindowTestNAME krita-image-KisMemoryWindowTest  ${kis_memory_window_test_SRCS})
add_test(KisMemoryWindowTest KisMemoryWindowTest)
ecm_mark_as_test(KisMemoryWindowTest)
target_link_libraries(KisMemoryWindowTest   Qt5::Test)

########### next target ###############
set(kis_swapped_data_store_test_SRCS kis_swapped_data_store_test.cpp ../kis_tile_data.cc)
add_executable(KisSwappedDataStoreTestNAME krita-image-KisSwappedDataStoreTest  ${kis_swapped_data_store_test_SRCS})
add_test(KisSwappedDataStoreTest KisSwappedDataStoreTest)
ecm_mark_as_test(KisSwappedDataStoreTest)
target_link_libraries(KisSwappedDataStoreTest   kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
set(kis_tile_data_store_test_SRCS kis_tile_data_store_test.cpp ../kis_tile_data.cc)
add_executable(KisTileDataStoreTestNAME krita-image-KisTileDataStoreTest  ${kis_tile_data_store_test_SRCS})
add_test(KisTileDataStoreTest KisTileDataStoreTest)
ecm_mark_as_test(KisTileDataStoreTest)
target_link_libraries(KisTileDataStoreTest   kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
set(kis_store_limits_test_SRCS kis_store_limits_test.cpp ../kis_tile_data.cc )
add_executable(KisStoreLimitsTestNAME krita-image-KisStoreLimitsTest  ${kis_store_limits_test_SRCS})
add_test(KisStoreLimitsTest KisStoreLimitsTest)
ecm_mark_as_test(KisStoreLimitsTest)
target_link_libraries(KisStoreLimitsTest   kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
set(kis_tile_data_pooler_test_SRCS kis_tile_data_pooler_test.cpp ../kis_tile_data.cc ../kis_tile_data_pooler.cc )
add_executable(KisTileDataPoolerTestNAME krita-image-KisTileDataPoolerTest  ${kis_tile_data_pooler_test_SRCS})
add_test(KisTileDataPoolerTest KisTileDataPoolerTest)
ecm_mark_as_test(KisTileDataPoolerTest)
target_link_libraries(KisTileDataPoolerTest   kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

