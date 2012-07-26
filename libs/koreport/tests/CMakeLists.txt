set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/
    ${CMAKE_SOURCE_DIR}/libs/koreport
    ${CMAKE_SOURCE_DIR}/libs/koreport/common
    ${CMAKE_SOURCE_DIR}/libs/koreport/renderer
    ${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
    ${CMAKE_SOURCE_DIR}/libs/koproperty
)


########## next target ###############

set(KoReportTest_SRCS KoReportTest.cpp)

kde4_add_unit_test(KoReportTest TESTNAME KoReportTest  ${KoReportTest_SRCS})

target_link_libraries(KoReportTest koreport ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########## next target ###############
