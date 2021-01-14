include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_tests(
    kis_meta_data_test.cpp
    NAME_PREFIX "libs-metadata-"
    LINK_LIBRARIES kritametadata Qt5::Test)
