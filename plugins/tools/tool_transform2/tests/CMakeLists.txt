include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

include(KritaAddBrokenUnitTest)
macro_add_unittest_definitions()

########### next target ###############

ecm_add_test(test_save_load_transform_args.cpp
    NAME_PREFIX plugins-tooltransform-
    LINK_LIBRARIES kritatooltransform kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(test_animated_transform_parameters.cpp
    NAME_PREFIX plugins-tooltransform-
    LINK_LIBRARIES kritatooltransform kritaui kritaimage Qt5::Test)
