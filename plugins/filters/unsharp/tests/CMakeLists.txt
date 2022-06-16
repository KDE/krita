include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

if (APPLE)
    include(KritaAddBrokenUnitTest)

    krita_add_broken_unit_tests(
        kis_unsharp_mask_test.cpp
        NAME_PREFIX "krita-filters-unsharp-"
        LINK_LIBRARIES kritaui Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST}
        )

    macos_test_fixrpath(${BROKEN_TESTS})

else (APPLE)

ecm_add_tests(
    kis_unsharp_mask_test.cpp
    NAME_PREFIX "krita-filters-unsharp-"
    LINK_LIBRARIES kritaui Qt5::Test)

endif()
