########### next target ###############
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)


if (APPLE)

    krita_add_broken_unit_tests(
        kis_sensors_test.cpp
        kis_linked_pattern_manager_test.cpp

        NAME_PREFIX "plugins-libpaintop-"
        LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST}
        )

    macos_test_fixrpath(${BROKEN_TESTS})

else (APPLE)
    ecm_add_test(kis_sensors_test.cpp
        NAME_PREFIX "plugins-libpaintop-"
        LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)

    krita_add_broken_unit_test(kis_linked_pattern_manager_test.cpp
        NAME_PREFIX "plugins-libpaintop-"
        LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)


endif()
