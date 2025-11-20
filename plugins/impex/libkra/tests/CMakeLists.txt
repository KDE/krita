kis_add_tests(
    kis_kra_loader_test.cpp
    LINK_LIBRARIES kritaui kritalibkra kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )


kis_add_test(
    kis_kra_saver_test.cpp
    LINK_LIBRARIES kritaui kritalibkra kritatransformmaskstubs
    NAME_PREFIX "plugins-impex-"
    )
