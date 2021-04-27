include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

if (APPLE)

    krita_add_broken_unit_tests(
        kis_kra_loader_test.cpp
        kis_kra_saver_test.cpp
        LINK_LIBRARIES kritaui kritalibkra Qt5::Test
        NAME_PREFIX "plugins-impex-"
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST}
    )

    macos_test_fixrpath(${BROKEN_TESTS})

else (APPLE)
    ecm_add_test(
        kis_kra_loader_test.cpp
        kis_kra_saver_test.cpp
        LINK_LIBRARIES kritaui kritalibkra Qt5::Test
        NAME_PREFIX "plugins-impex-"
        )

endif()
