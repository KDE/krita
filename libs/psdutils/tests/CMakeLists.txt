include_directories(
    ${CMAKE_SOURCE_DIR}/sdk/tests
    ${CMAKE_SOURCE_DIR}/libs/psdutils
)

macro_add_unittest_definitions()

if (WIN32)
    set(PSD_TEST_LIBS kritapsdutils Qt5::Test ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    set(PSD_TEST_LIBS kritapsdutils Qt5::Test)
endif (WIN32)

ecm_add_tests(
    psd_utils_test.cpp
    compression_test.cpp
    NAME_PREFIX "kritapsdutils-"
    LINK_LIBRARIES ${PSD_TEST_LIBS})
