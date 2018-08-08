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
    NAME_PREFIX "libs-image-tiles3-"
    LINK_LIBRARIES kritaimage Qt5::Test)

set_tests_properties(libs-image-tiles3-kis_low_memory_tests PROPERTIES TIMEOUT 180)

ecm_add_test(
    kis_chunk_allocator_test.cpp ../swap/kis_chunk_allocator.cpp
    TEST_NAME libs-image-KisChunkAllocatorTest
    LINK_LIBRARIES kritaglobal Qt5::Test)

ecm_add_test(
    kis_memory_window_test.cpp ../swap/kis_memory_window.cpp
    TEST_NAME libs-image-KisMemoryWindowTest
    LINK_LIBRARIES kritaglobal Qt5::Test)

########### next target ###############
krita_add_broken_unit_test(kis_swapped_data_store_test.cpp ../kis_tile_data.cc
    TEST_NAME libs-image-KisSwappedDataStoreTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
krita_add_broken_unit_test(kis_tile_data_store_test.cpp ../kis_tile_data.cc
    TEST_NAME libs-image-KisTileDataStoreTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
krita_add_broken_unit_test(kis_store_limits_test.cpp ../kis_tile_data.cc
    TEST_NAME libs-image-KisStoreLimitsTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})

########### next target ###############
krita_add_broken_unit_test(kis_tile_data_pooler_test.cpp ../kis_tile_data.cc ../kis_tile_data_pooler.cc
    TEST_NAME libs-image-KisTileDataPoolerTest
    LINK_LIBRARIES kritaimage Qt5::Test ${Boost_SYSTEM_LIBRARY})
