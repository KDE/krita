set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${FLAKE_INCLUDES}
    ${KOREPORT_INCLUDES}
    
)


########## next target ###############

set(KoReportTest_SRCS KoReportTest.cpp)

add_executable(KoReportTestNAME KoReportTest  ${KoReportTest_SRCS})
add_test(KoReportTest KoReportTest)
ecm_mark_as_test(KoReportTest)

target_link_libraries(KoReportTest ${KOREPORT_LIBS}  Qt5::Test)

########## next target ###############
