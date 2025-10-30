
include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "libs-global-")

kis_add_tests(KisSignalAutoConnectionTest.cpp
    KisSignalCompressorTest.cpp
    KisForestTest.cpp
    KisRectsGridTest.cpp
    KisLazyStorageTest.cpp
    KisValueCacheTest.cpp
    KisHistoryListTest.cpp
    LINK_LIBRARIES kritaglobal kritatestsdk
    )

