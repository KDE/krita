include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

ecm_add_tests(KisSharedThreadPoolAdapterTest.cpp
    KisSignalAutoConnectionTest.cpp
    KisSignalCompressorTest.cpp
    KisForestTest.cpp
    NAME_PREFIX "libs-global-"
    LINK_LIBRARIES kritaglobal Qt5::Test
    TARGET_NAMES_VAR OK_TESTS
    ${MACOS_GUI_TEST})

macos_test_fixrpath(${OK_TESTS})
