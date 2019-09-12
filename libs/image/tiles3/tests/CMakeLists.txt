set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   
    ${CMAKE_BINARY_DIR}/libs/image
)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)


macro_add_unittest_definitions()

krita_add_broken_unit_tests(
    kis_tiled_data_manager_test.cpp
    
    LINK_LIBRARIES kritaimage Qt5::Test
    NAME_PREFIX "libs-image-tiles3-")

ecm_add_tests(
    kis_low_memory_tests.cpp
    kis_lockless_stack_test.cpp
    kis_chunk_allocator_test.cpp
    kis_memory_window_test.cpp
    kis_store_limits_test.cpp
    kis_swapped_data_store_test.cpp
    kis_tile_data_store_test.cpp
    kis_tile_data_pooler_test.cpp

    LINK_LIBRARIES kritaimage Qt5::Test
    NAME_PREFIX "libs-image-tiles3-")

set_tests_properties(libs-image-tiles3-kis_low_memory_tests PROPERTIES TIMEOUT 180)
