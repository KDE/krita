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
    TestNumberStyle.cpp
    TestKoElementReference.cpp
    NAME_PREFIX "libs-odf-"
    LINK_LIBRARIES kritaodf Qt5::Test)
