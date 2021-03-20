include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
macro_add_unittest_definitions()

ecm_add_tests(
    #TestKrita.cpp
    TestChannel.cpp
    TestDocument.cpp
    TestNode.cpp
    TestFilter.cpp
    TestManagedColor.cpp
    TestNotifier
    NAME_PREFIX "libs-libkis-"
    LINK_LIBRARIES kritalibkis Qt5::Test)
