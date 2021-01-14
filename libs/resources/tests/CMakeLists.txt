include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

add_definitions(-DFILES_DEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/data/")

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
    TestStorageFilterProxyModel
    TestResourceServer
    TestTagResourceModel
    NAME_PREFIX "libs-kritaresources-"
    LINK_LIBRARIES kritaglobal kritapigment kritaplugin kritaresources kritawidgets kritaversion KF5::ConfigCore Qt5::Sql Qt5::Test)
