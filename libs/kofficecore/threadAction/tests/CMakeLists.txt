if(KDE4_BUILD_TESTS)

include_directories(
    ${KOFFICECORE_INCLUDES}
    ${KDE4_INCLUDE_DIR}/threadweaver/
)

########### next target ###############

set(TestAction_SRCS TestAction.cpp )

kde4_automoc(${TestAction_SRCS})

kde4_add_executable(TestAction RUN_UNINSTALLED ${TestAction_SRCS})

target_link_libraries(TestAction kofficecore ${QT_QTTEST_LIBRARY} )

add_test(TestAction TestAction)

########### end ###############

endif(KDE4_BUILD_TESTS)
