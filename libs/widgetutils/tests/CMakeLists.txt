macro_add_unittest_definitions()

include_directories (
    ${CMAKE_SOURCE_DIR}/libs/widgetutils
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

include(ECMAddTests)

if (APPLE)
    include(KritaAddBrokenUnitTest)

    krita_add_broken_unit_tests(
        KisActionsSnapshotTest.cpp
        NAME_PREFIX "libs-widgetutils-"
        LINK_LIBRARIES kritawidgetutils kritaimage Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST})

    macos_test_fixrpath(${BROKEN_TESTS})


else (APPLE)
    ecm_add_test(
        KisActionsSnapshotTest.cpp
        NAME_PREFIX "libs-widgetutils-"
        LINK_LIBRARIES kritawidgetutils kritaimage Qt5::Test
    )

endif()

ecm_add_tests(
    KoPropertiesTest.cpp
    kis_simple_math_parser_test.cpp # FIXME this test should be in the ui directory
    TestKoProgressUpdater.cpp
    TestSqueezedComboBox.cpp
    kis_parse_spin_boxes_test.cpp
    NAME_PREFIX "libs-widgetutils-"
    LINK_LIBRARIES kritawidgetutils kritaimage Qt5::Test
    TARGET_NAMES_VAR OK_TESTS
    ${MACOS_GUI_TEST}
)

ki18n_wrap_ui(DialogStateSaver_SRCS
    dialogsavertestwidget.ui
)

ecm_add_test(
    KisDialogStateSaverTest.cpp
    ${DialogStateSaver_SRCS}
    TEST_NAME KisDialogStateSaverTest
    LINK_LIBRARIES kritawidgetutils Qt5::Test KF5::ConfigCore
    NAME_PREFIX "libs-widgetutils-"
    ${MACOS_GUI_TEST})


macos_test_fixrpath(${OK_TESTS} KisDialogStateSaverTest)

