include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

kis_add_tests(KisSharedThreadPoolAdapterTest.cpp
    KisSignalAutoConnectionTest.cpp
    KisSignalCompressorTest.cpp
    KisForestTest.cpp
    KisRectsGridTest.cpp
    KisLazyStorageTest.cpp
    NAME_PREFIX "libs-global-"
    LINK_LIBRARIES kritaglobal Qt5::Test
    )

