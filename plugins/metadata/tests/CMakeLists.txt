##### Tests that currently fail and should be fixed #####

# Works under Linux but does not work under Windows
krita_add_broken_unit_test( kis_exif_test.cpp
    TEST_NAME KisExifTest
    LINK_LIBRARIES kritametadata kritaui kritatestsdk
    )
