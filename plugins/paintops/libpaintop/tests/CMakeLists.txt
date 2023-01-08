########### next target ###############


kis_add_test(kis_sensors_test.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop kritatestsdk)

krita_add_broken_unit_test(kis_linked_pattern_manager_test.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop kritatestsdk)
