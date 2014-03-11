set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_SOURCE_DIR}/krita/plugins/paintops/resourcemanager)

set(KoResourceBundle_test_SRCS KoResourceBundle_test.cpp )
kde4_add_unit_test(KoResourceBundleTest ${KoResourceBundle_test_SRCS})
target_link_libraries(KoResourceBundleTest koplugin kritaresourcemanager kritaui kritalibpaintop ${QT_QTTEST_LIBRARY}) 
########### end ###############
