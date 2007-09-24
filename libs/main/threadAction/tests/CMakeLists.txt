set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${KOMAIN_INCLUDES}
    ${KDE4_INCLUDE_DIR}/threadweaver/
)

########### next target ###############

set(TestAction_SRCS TestAction.cpp )


kde4_add_unit_test(TestAction TESTNAME libs-komain-threadAction-TestAction ${TestAction_SRCS})

target_link_libraries(TestAction komain ${QT_QTTEST_LIBRARY} )

########### end ###############
