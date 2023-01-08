macro_add_unittest_definitions()

if(APPLE)
    krita_add_broken_unit_test(
        KisHeifTest.cpp
        TEST_NAME KisHeifTest
        LINK_LIBRARIES kritaui kritatestsdk
        NAME_PREFIX "plugins-impex-"
    )

else(APPLE)

    kis_add_test(
        KisHeifTest.cpp
        TEST_NAME KisHeifTest
        LINK_LIBRARIES kritaui kritatestsdk
        NAME_PREFIX "plugins-impex-"
    )


endif()
