macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
#add_subdirectory(filedialogtester)

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

ecm_add_tests(
    zoomhandler_test.cpp
    zoomcontroller_test.cpp
    KoAnchorSelectionWidgetTest.cpp
    TestKoZoomAction.cpp
    TestKisTagSelectionWidget.cpp
    NAME_PREFIX "libs-widgets-"
    LINK_LIBRARIES kritawidgets Qt5::Test
    TARGET_NAMES_VAR OK_TESTS
    ${MACOS_GUI_TEST})

macos_test_fixrpath(${OK_TESTS})
