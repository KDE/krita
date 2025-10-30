########### next target ###############

kis_add_test_maybe_broken(SvgTextCursorTest.cpp
    TEST_NAME SvgTextCursorTest
    LINK_LIBRARIES krita_tool_svgtext_static kritaflake kritaui kritatestsdk
    BROKEN_IF ${WIN32})
