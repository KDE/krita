include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_tests(KisSharedThreadPoolAdapterTest.cpp
    NAME_PREFIX libs-global
    LINK_LIBRARIES kritaglobal Qt5::Test)
