set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
  
include_directories(${CMAKE_SOURCE_DIR}/libs/image/metadata
                    ${CMAKE_SOURCE_DIR}/sdk/tests )
include(ECMAddTests)

macro_add_unittest_definitions()
qt5_add_resources(kis_node_view_test_SRCS ${krita_QRCS})
ecm_add_test(${kis_node_view_test_SRCS}
    TEST_NAME TestNodeView
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-dockers-layerdocker-")

