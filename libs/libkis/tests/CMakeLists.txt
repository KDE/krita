include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
macro_add_unittest_definitions()



if (APPLE)
    # Broken unit test.app can be run if installed
    krita_add_broken_unit_tests(
#        TestKrita.cpp
        TestChannel.cpp
        TestDocument.cpp
        TestNode.cpp
        TestFilter.cpp
        TestManagedColor.cpp
        TestNotifier
        NAME_PREFIX "libs-libkis-"
        LINK_LIBRARIES kritalibkis Qt5::Test
        TARGET_NAMES_VAR OK_TESTS
        GUI)

    macos_test_fixrpath(${OK_TESTS})

else (APPLE)
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

endif()

