
if(KDE4_BUILD_TESTS)

include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )

kde4_automoc(${TestPosition_test_SRCS})

kde4_add_executable(TestPosition RUN_UNINSTALLED ${TestPosition_test_SRCS})

target_link_libraries(TestPosition  ${KDE4_KDECORE_LIBS} flake ${QT_QTTEST_LIBRARY})

endif(KDE4_BUILD_TESTS)
