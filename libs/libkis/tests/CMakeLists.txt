
include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "libs-libkis-")

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
    LINK_LIBRARIES kritalibkis kritatestsdk
    )

