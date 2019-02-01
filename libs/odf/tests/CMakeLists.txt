set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

ecm_add_tests(
    TestKoGenStyles.cpp
    TestOdfSettings.cpp
    TestKoOdfLoadingContext.cpp
    TestStorage.cpp
    NAME_PREFIX "libs-odf-"
    LINK_LIBRARIES kritaodf KF5::I18n Qt5::Test)

ecm_add_tests(
    TestXmlWriter.cpp
    kodomtest.cpp
    TestKoUnit.cpp
    TestKoElementReference.cpp
    NAME_PREFIX "libs-odf-"
    LINK_LIBRARIES kritaodf Qt5::Test)


include(KritaAddBrokenUnitTest)

krita_add_broken_unit_test(
    TestNumberStyle.cpp
    TEST_NAME TestNumberStyle
    LINK_LIBRARIES kritaodf Qt5::Test
    NAME_PREFIX "libs-odf-")

