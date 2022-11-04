include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
macro_add_unittest_definitions()


kis_add_tests(
    #TestKrita.cpp
    TestChannel.cpp
    TestDocument.cpp
    TestNode.cpp
    TestVectorLayer.cpp
    TestShape.cpp
    TestFilter.cpp
    TestManagedColor.cpp
    TestNotifier.cpp
    NAME_PREFIX "libs-libkis-"
    LINK_LIBRARIES kritalibkis Qt5::Test
    )

