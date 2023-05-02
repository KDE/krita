include(KritaAddBrokenUnitTest)

kis_add_tests(
    TestPerspectiveBasedAssistantHelper.cpp
    TestAssistants.cpp
    TestEllipseInPolygon.cpp
    NAME_PREFIX "plugins-assistants-Assistants-"
    LINK_LIBRARIES kritaassistanttool_static kritatestsdk
    )
