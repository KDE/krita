set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( 
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${CMAKE_SOURCE_DIR}/krita/image
    ${CMAKE_SOURCE_DIR}/krita/ui
    ${CMAKE_SOURCE_DIR}/krita/plugins/paintops/libbrush
    ${CMAKE_SOURCE_DIR}/krita/plugins/paintops/libpaintop
    ${CMAKE_SOURCE_DIR}/krita/sdk/tests 
    ${CMAKE_SOURCE_DIR}/krita/plugins/paintops/resourcemanager)

macro_add_unittest_definitions()

set(ResourceBundleTest_SRCS ResourceBundleTest.cpp ../resourcebundle.cpp ../resourcebundle_manifest.cpp)
add_executable(ResourceBundleTestNAME krita-resourcemanager-ResourceBundleTest ${ResourceBundleTest_SRCS})
add_test(ResourceBundleTest ResourceBundleTest)
ecm_mark_as_test(ResourceBundleTest)
target_link_libraries(ResourceBundleTest kritaui kritalibbrush kritalibpaintop Qt5::Test )
