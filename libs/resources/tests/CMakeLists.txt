set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_tests(
    TestResourceCacheDb
    TestResourceLoaderRegistry
    TestResourceLocator
    TestResourceStorage
    NAME_PREFIX "libs-kritaresources-"
    LINK_LIBRARIES kritaresources KF5::ConfigCore Qt5::Sql Qt5::Test)
