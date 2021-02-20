macro_add_unittest_definitions()

#add_subdirectory(filedialogtester)

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

ecm_add_tests(
    zoomhandler_test.cpp
    zoomcontroller_test.cpp
    KoAnchorSelectionWidgetTest.cpp
    TestKoZoomAction.cpp
    NAME_PREFIX "libs-widgets-"
    LINK_LIBRARIES kritawidgets Qt5::Test)
