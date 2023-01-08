macro_add_unittest_definitions()

kis_add_tests(
    TestPerspectiveBasedAssistantHelper.cpp
    TestAssistants.cpp
    NAME_PREFIX "plugins-assistants-Assistants-"
    LINK_LIBRARIES kritaassistanttool_static kritatestsdk
    )
