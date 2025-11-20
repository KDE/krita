set(PSD_TEST_LIBS kritapsdutils kritatestsdk)

kis_add_tests(
    psd_utils_test.cpp
    compression_test.cpp
    psd_cos_parser_test.cpp
    NAME_PREFIX "kritapsdutils-"
    LINK_LIBRARIES ${PSD_TEST_LIBS})
