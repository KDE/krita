macro_add_unittest_definitions()

#add_subdirectory(filedialogtester)

kis_add_tests(
    zoomhandler_test.cpp
    zoomcontroller_test.cpp
    KoAnchorSelectionWidgetTest.cpp
    TestKoZoomAction.cpp
    TestKisTagSelectionWidget.cpp
    NAME_PREFIX "libs-widgets-"
    LINK_LIBRARIES kritawidgets kritatestsdk
    )
