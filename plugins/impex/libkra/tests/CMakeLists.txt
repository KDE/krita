macro_add_unittest_definitions()

kis_add_tests(
    kis_kra_loader_test.cpp
    kis_kra_saver_test.cpp
    LINK_LIBRARIES kritaui kritalibkra kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )
