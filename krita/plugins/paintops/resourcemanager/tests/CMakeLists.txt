set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_SOURCE_DIR}/krita/plugins/paintops/resourcemanager)

set(KoResourceBundleManager_test_SRCS KoResourceBundleManager_test.cpp )
kde4_add_unit_test(KoResourceBundleManagerTest ${KoResourceBundleManager_test_SRCS})
target_link_libraries(KoResourceBundleManagerTest koplugin kritaresourcemanager kritaui kritalibpaintop ${QT_QTTEST_LIBRARY})
########### end ############### 
