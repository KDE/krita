
include(KritaAddBrokenUnitTest)

kis_add_tests(KisSignalAutoConnectionTest.cpp
    KisSignalCompressorTest.cpp
    KisForestTest.cpp
    KisRectsGridTest.cpp
    KisLazyStorageTest.cpp
    KisValueCacheTest.cpp
    NAME_PREFIX "libs-global-"
    LINK_LIBRARIES kritaglobal kritatestsdk
    )

