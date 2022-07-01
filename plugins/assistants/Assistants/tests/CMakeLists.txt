include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/sdk/tests ${CMAKE_CURRENT_BINARY_DIR}/..)

macro_add_unittest_definitions()

kis_add_tests(
    TestPerspectiveBasedAssistantHelper.cpp
    NAME_PREFIX "plugins-assistants-Assistants-"
    LINK_LIBRARIES ${KDE4_KDEUI_LIBS} kritaassistanttool kritaui kritaflake Qt5::Test
    TARGET_NAMES_VAR OK_TESTS
    ${MACOS_GUI_TEST})

macos_test_fixrpath(${OK_TESTS})
