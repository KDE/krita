include_directories(   
    ${CMAKE_BINARY_DIR}/libs/image
)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)

include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

ecm_add_tests(
    kis_tiled_data_manager_test.cpp
    kis_low_memory_tests.cpp
    kis_lockless_stack_test.cpp
    kis_chunk_allocator_test.cpp
    kis_memory_window_test.cpp
    kis_store_limits_test.cpp
    kis_swapped_data_store_test.cpp
    kis_tile_data_store_test.cpp
    kis_tile_data_pooler_test.cpp
    LINK_LIBRARIES kritaimage Qt5::Test
    NAME_PREFIX "libs-image-tiles3-"
    TARGET_NAMES_VAR OK_TESTS
    ${MACOS_GUI_TEST})

set_tests_properties(libs-image-tiles3-kis_low_memory_tests PROPERTIES TIMEOUT 180)

macos_test_fixrpath(${OK_TESTS})
