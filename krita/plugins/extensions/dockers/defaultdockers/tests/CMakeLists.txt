include_directories( ${CMAKE_SOURCE_DIR}/krita/sdk/tests ${CMAKE_CURRENT_BINARY_DIR}/..)

macro_add_unittest_definitions()

########### next target ###############

set(kis_node_juggler_compressed_test_SRCS kis_node_juggler_compressed_test.cpp )
kde4_add_unit_test(KisNodeJugglerCompressedTest TESTNAME krita-image-BaseNodeTest ${kis_node_juggler_compressed_test_SRCS})
target_link_libraries(KisNodeJugglerCompressedTest kritadefaultdockers kritaimage kritaui Qt5::Test)
