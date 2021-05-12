include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include(ECMAddTests)
macro_add_unittest_definitions()

include(KritaAddBrokenUnitTest)

if (APPLE)

    krita_add_broken_unit_tests(
        kis_svg_test.cpp

        LINK_LIBRARIES kritaui Qt5::Test
        NAME_PREFIX "plugins-impex-"
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST}
    )

    macos_test_fixrpath(${BROKEN_TESTS})

else (APPLE)
    krita_add_broken_unit_test(
        kis_svg_test.cpp
        TEST_NAME KisSvgTest
        LINK_LIBRARIES kritaui Qt5::Test
        NAME_PREFIX "plugins-impex-"
        )

endif()
