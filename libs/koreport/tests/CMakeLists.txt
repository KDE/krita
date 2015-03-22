set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${FLAKE_INCLUDES}
    ${KOREPORT_INCLUDES}
    ${QT_INCLUDES}
)


########## next target ###############

set(KoReportTest_SRCS KoReportTest.cpp)

kde4_add_unit_test(KoReportTest TESTNAME KoReportTest  ${KoReportTest_SRCS})

target_link_libraries(KoReportTest ${KOREPORT_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########## next target ###############
