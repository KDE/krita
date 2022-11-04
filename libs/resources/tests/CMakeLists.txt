include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
macro_add_unittest_definitions()

kis_add_tests(
    TestResourceCacheDb.cpp
    TestResourceLoaderRegistry.cpp
    TestResourceLocator.cpp
    TestResourceStorage.cpp
    TestFolderStorage.cpp
    TestMemoryStorage.cpp
    TestTag.cpp
    TestBundleStorage.cpp
    TestResourceModel.cpp
    TestTagFilterResourceProxyModel.cpp
    TestTagModel.cpp
    TestResourceTypeModel.cpp
    TestStorageModel.cpp
    TestResourceSearchBoxFilter.cpp
    TestStorageFilterProxyModel.cpp
    TestTagResourceModel.cpp
    NAME_PREFIX "libs-kritaresources-"
    LINK_LIBRARIES kritaglobal kritapigment kritaplugin kritaresources kritawidgets kritaversion KF5::ConfigCore Qt5::Sql Qt5::Test
    )
