if(KDE4_BUILD_TESTS)

#add_definitions(-DCOMPILING_TESTS)

include_directories(
#    ${KOFFICECORE_INCLUDES}
#    ${FLAKE_INCLUDES}
    ${KOTEXT_INCLUDES}
#    ${KDE4_INCLUDES}
    ${QT_INCLUDES})

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
kde4_automoc(${TestStyles_test_SRCS})
kde4_add_executable(TestStyles RUN_UNINSTALLED ${TestStyles_test_SRCS})
target_link_libraries(TestStyles kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

endif(KDE4_BUILD_TESTS)
