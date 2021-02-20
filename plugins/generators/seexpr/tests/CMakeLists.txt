include_directories(
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

macro_add_unittest_definitions()

ecm_add_tests(
    kis_seexpr_generator_test.cpp

    NAME_PREFIX "plugins-generators-"
    LINK_LIBRARIES kritaimage Qt5::Test)
