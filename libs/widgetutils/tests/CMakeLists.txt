kis_add_tests(
    KisActionsSnapshotTest.cpp
    KoPropertiesTest.cpp
    kis_simple_math_parser_test.cpp # FIXME this test should be in the ui directory
    TestKoProgressUpdater.cpp
    TestSqueezedComboBox.cpp
    kis_parse_spin_boxes_test.cpp
    NAME_PREFIX "libs-widgetutils-"
    LINK_LIBRARIES kritawidgetutils kritaimage kritatestsdk
)

ki18n_wrap_ui(DialogStateSaver_SRCS
    dialogsavertestwidget.ui
)

kis_add_test(
    KisDialogStateSaverTest.cpp
    ${DialogStateSaver_SRCS}
    TEST_NAME KisDialogStateSaverTest
    LINK_LIBRARIES kritawidgetutils kritatestsdk KF${KF_MAJOR}::ConfigCore
    NAME_PREFIX "libs-widgetutils-"
    )

