set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_tests(
    #TestKrita.cpp
    TestChannel.cpp
    TestDocument.cpp
    TestNode.cpp
    TestFilter.cpp
    TestManagedColor.cpp
    NAME_PREFIX "libs-kritalibkis-"
    LINK_LIBRARIES kritalibkis Qt5::Test)
