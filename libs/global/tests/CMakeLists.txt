include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

ecm_add_test(KisSharedThreadPoolAdapterTest.cpp
    TEST_NAME KisSharedThreadPoolAdapter
    LINK_LIBRARIES kritaglobal Qt5::Test)
