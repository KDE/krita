include_directories(${CMAKE_SOURCE_DIR}/sdk/tests )

include(ECMAddTests)

macro_add_unittest_definitions()

include(KritaAddBrokenUnitTest)

##### Tests that currently fail and should be fixed #####

# Works under Linux but does not work under Windows
krita_add_broken_unit_test( kis_exif_test.cpp
    TEST_NAME KisExifTest
    LINK_LIBRARIES kritametadata kritaui Qt5::Test
    NAME_PREFIX "plugins-metadata-"
    ${MACOS_GUI_TEST})

macos_test_fixrpath(
    ${BROKEN_TESTS}
    KisExifTest
    )
