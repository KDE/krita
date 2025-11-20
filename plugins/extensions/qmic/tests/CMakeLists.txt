kis_add_test(kis_qmic_tests.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../kis_qmic_simple_convertor.cpp
    TEST_NAME kis_qmic_tests
    LINK_LIBRARIES kritaimage kritaqmicinterface kritatestsdk
    NAME_PREFIX "plugins-extensions-qmic-")

