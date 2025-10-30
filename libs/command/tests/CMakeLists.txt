
include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "libs-command-")

kis_add_tests(
    TestKUndo2Stack.cpp

    LINK_LIBRARIES kritacommand kritatestsdk
    )

