add_definitions(-DFILES_DEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/data/")

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_tests(
    TestResourceCacheDb
    TestResourceLoaderRegistry
    TestResourceLocator
    TestResourceStorage
    TestFolderStorage
    TestMemoryStorage
    TestTag
    TestBundleStorage
    TestResourceModel
    TestTagFilterResourceProxyModel
    TestTagModel
    TestResourceTypeModel
    TestStorageModel
    TestResourceSearchBoxFilter
    NAME_PREFIX "libs-kritaresources-"
    LINK_LIBRARIES kritaglobal kritaplugin kritaresources kritaversion KF5::ConfigCore Qt5::Sql Qt5::Test)
