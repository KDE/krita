include_directories(${CMAKE_SOURCE_DIR}/sdk/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_BINARY_DIR}/plugins/extensions/qmic
)

macro_add_unittest_definitions()

if (APPLE)
    include(KritaAddBrokenUnitTest)

    krita_add_broken_unit_test(kis_qmic_tests.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../kis_qmic_simple_convertor.cpp
        TEST_NAME kis_qmic_tests
        LINK_LIBRARIES kritaimage Qt5::Test
        NAME_PREFIX "plugins-extensions-qmic-"
        GUI)

    macos_test_fixrpath(kis_qmic_tests)

else (APPLE)


    ecm_add_test(kis_qmic_tests.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../kis_qmic_simple_convertor.cpp
        TEST_NAME kis_qmic_tests
        LINK_LIBRARIES kritaimage Qt5::Test
        NAME_PREFIX "plugins-extensions-qmic-")

endif()
