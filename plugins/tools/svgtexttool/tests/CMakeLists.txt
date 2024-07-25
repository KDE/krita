include(KritaAddBrokenUnitTest)

########### next target ###############

krita_add_broken_unit_test(SvgTextCursorTest.cpp
    TEST_NAME SvgTextCursorTest
    LINK_LIBRARIES krita_tool_svgtext_static kritaflake kritaui kritatestsdk
    NAME_PREFIX plugins-tools-svgtext-)
