include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/sdk/tests ${CMAKE_CURRENT_BINARY_DIR}/..)

macro_add_unittest_definitions()

kis_add_tests(
    TestPerspectiveBasedAssistantHelper.cpp
    TestAssistants.cpp
    NAME_PREFIX "plugins-assistants-Assistants-"
    LINK_LIBRARIES kritaassistanttool_static Qt5::Test
    )
