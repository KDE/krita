#add_subdirectory(filedialogtester)

kis_add_tests(
    zoomhandler_test.cpp
    KoAnchorSelectionWidgetTest.cpp
    TestKoZoomAction.cpp
    TestKisTagSelectionWidget.cpp
    TestKisPaletteModel.cpp
    NAME_PREFIX "libs-widgets-"
    LINK_LIBRARIES kritawidgets kritatestsdk
    )
