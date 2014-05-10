set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${KOMAIN_INCLUDES} 
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${CMAKE_SOURCE_DIR}/krita/image
    ${CMAKE_SOURCE_DIR}/krita/ui
    ${CMAKE_SOURCE_DIR}/krita/plugins/paintops/libbrush
    ${CMAKE_SOURCE_DIR}/krita/plugins/paintops/libpaintop
    ${CMAKE_SOURCE_DIR}/krita/sdk/tests 
    ${CMAKE_SOURCE_DIR}/krita/plugins/paintops/resourcemanager)

macro_add_unittest_definitions()

#set(KoResourceBundleManager_test_SRCS KoResourceBundleManager_test.cpp )
#kde4_add_unit_test(KoResourceBundleManagerTest ${KoResourceBundleManager_test_SRCS})
#target_link_libraries(KoResourceBundleManagerTest koplugin kritaresourcemanager kritaui kritalibpaintop ${QT_QTTEST_LIBRARY})

#set(KoXmlGeneratorTest_SRCS KoXmlGeneratorTest.cpp )
#kde4_add_unit_test(KoXmlGeneratorTest ${KoXmlGeneratorTest_SRCS})
#target_link_libraries(KoXmlGeneratorTest koplugin kritaresourcemanager kritaui kritalibpaintop ${QT_QTTEST_LIBRARY})

#set(KoXmlResourceBundleManifestTest_SRCS KoXmlResourceBundleManifestTest.cpp )
#kde4_add_unit_test(KoXmlResourceBundleManifestTest ${KoXmlResourceBundleManifestTest_SRCS})
#target_link_libraries(KoXmlResourceBundleManifestTest koplugin kritaresourcemanager kritaui kritalibpaintop ${QT_QTTEST_LIBRARY})

#set(KoXmlResourceBundleMetaTest_SRCS KoXmlResourceBundleMetaTest.cpp )
#kde4_add_unit_test(KoXmlResourceBundleMetaTest ${KoXmlResourceBundleMetaTest_SRCS})
#target_link_libraries(KoXmlResourceBundleMetaTest koplugin kritaresourcemanager kritaui kritalibpaintop ${QT_QTTEST_LIBRARY})

set(KoResourceBundleTest_SRCS KoResourceBundleTest.cpp ../KoResourceBundle.cpp ../KoXmlGenerator.cpp ../KoXmlResourceBundleMeta.cpp ../KoXmlResourceBundleManifest.cpp)
kde4_add_unit_test(KoResourceBundleTest ${KoResourceBundleTest_SRCS})
target_link_libraries(KoResourceBundleTest kritaui kritalibbrush kritalibpaintop ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})
########### end ###############
