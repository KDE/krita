
include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "libs-flake-")

kis_add_tests(
    KoGamutMaskTest.cpp
    LINK_LIBRARIES kritaflake kritaimage kritatestsdk
    )
