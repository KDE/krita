set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   
    ${CMAKE_BINARY_DIR}/libs/image
)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)


macro_add_unittest_definitions()

ecm_add_tests(
    kis_tiled_data_manager_test.cpp
    kis_low_memory_tests.cpp
    kis_lockless_stack_test.cpp
    NAME_PREFIX "krita-image-tiles3-"
    LINK_LIBRARIES kritaimage Qt5::Test)

set_tests_properties(krita-image-tiles3-kis_low_memory_tests PROPERTIES TIMEOUT 180)

########### next target ###############
#kisdoc
#set(kis_tile_compressors_test_SRCS kis_tile_compressors_test.cpp )
#kde4_add_unit_test(KisTileCompressorsTest TEST_NAME krita-image-KisTileCompressorsTest  ${kis_tile_compressors_test_SRCS})
#target_LINK_LIBRARIES(KisTileCompressorsTest   kritaodf kritaimage Qt5::Test)

########### next target ###############
#kisdoc
#set(kis_compression_tests_SRCS kis_compression_tests.cpp )
#kde4_add_unit_test(KisCompressionTests TEST_NAME krita-image-KisCompressionTests  ${kis_compression_tests_SRCS})
#target_LINK_LIBRARIES(KisCompressionTests   kritaimage Qt5::Test)

ecm_add_test(
    kis_memory_pool_test.cpp
    TEST_NAME krita-image-KisMemoryPoolTest
    LINK_LIBRARIES kritaglobal Qt5::Test)

ecm_add_test(
    kis_chunk_allocator_test.cpp ../swap/kis_chunk_allocator.cpp
    TEST_NAME krita-image-KisChunkAllocatorTest
    LINK_LIBRARIES kritaglobal Qt5::Test)

ecm_add_test(
    kis_memory_window_test.cpp ../swap/kis_memory_window.cpp
    TEST_NAME krita-image-KisMemoryWindowTest
    LINK_LIBRARIES kritaglobal Qt5::Test)

########### next target ###############
krita_add_broken_unit_test(kis_swapped_data_store_test.cpp ../kis_tile_data.cc
    TEST_NAME krita-image-KisSwappedDataStoreTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
krita_add_broken_unit_test(kis_tile_data_store_test.cpp ../kis_tile_data.cc
    TEST_NAME krita-image-KisTileDataStoreTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
krita_add_broken_unit_test(kis_store_limits_test.cpp ../kis_tile_data.cc
    TEST_NAME krita-image-KisStoreLimitsTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
krita_add_broken_unit_test(kis_tile_data_pooler_test.cpp ../kis_tile_data.cc ../kis_tile_data_pooler.cc
    TEST_NAME krita-image-KisTileDataPoolerTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})
