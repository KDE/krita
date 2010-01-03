set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/../ctlcolorspaces/\\"")

########### next target ###############
set(ko_ctl_color_space_info_test_SRCS KoCtlColorSpaceInfoTest.cpp ../KoCtlColorSpaceInfo.cpp ../KoCtlParser.cpp ../KoCtlAccumulator.cpp )

kde4_add_unit_test(ko_ctl_color_space_info_test TESTNAME krita-cs-ko_ctl_color_space_info_test ${ko_ctl_color_space_info_test_SRCS})

target_link_libraries(ko_ctl_color_space_info_test  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} pigmentcms ${OPENCTL_LDFLAGS})

########### next target ###############
set(ko_ctl_channel_test_SRCS KoCtlChannelTest.cpp ../KoCtlChannel.cpp )

kde4_add_unit_test(ko_ctl_channel_test TESTNAME krita-cs-ko_ctl_channel_test ${ko_ctl_channel_test_SRCS})

target_link_libraries(ko_ctl_channel_test  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} pigmentcms)

########### next target ###############
set(ko_rgb_32f_test_SRCS KoRgb32fTest.cpp )

kde4_add_unit_test(ko_rgb_32f_test TESTNAME krita-cs-ko_rgb_32f_test ${ko_rgb_32f_test_SRCS})

target_link_libraries(ko_rgb_32f_test  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} pigmentcms)

########### next target ###############
set(ko_rgb_16f_test_SRCS KoRgb16fTest.cpp )

kde4_add_unit_test(ko_rgb_16f_test TESTNAME krita-cs-ko_rgb_16f_test ${ko_rgb_16f_test_SRCS})

target_link_libraries(ko_rgb_16f_test  ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} pigmentcms)

