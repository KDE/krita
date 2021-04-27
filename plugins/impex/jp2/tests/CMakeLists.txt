include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

if (APPLE)

    krita_add_broken_unit_test(KisJP2Test.cpp
        TEST_NAME KisJP2Test
        LINK_LIBRARIES kritaui Qt5::Test
        NAME_PREFIX "plugins-impex-"
        ${MACOS_GUI_TEST}
    )

    macos_test_fixrpath(KisJP2Test)

else (APPLE)
    krita_add_broken_unit_test(KisJP2Test.cpp
        TEST_NAME KisJP2Test
        LINK_LIBRARIES kritaui Qt5::Test
        NAME_PREFIX "plugins-impex-"
    )

endif()
