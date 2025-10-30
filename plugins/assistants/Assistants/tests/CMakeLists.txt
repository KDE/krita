include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-assistants-Assistants-")

kis_add_tests(
    TestPerspectiveBasedAssistantHelper.cpp
    TestAssistants.cpp
    LINK_LIBRARIES kritaassistanttool_static kritatestsdk
    )
