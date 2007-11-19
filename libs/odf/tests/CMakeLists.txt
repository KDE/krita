set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOODF_INCLUDES} )

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
kde4_add_unit_test(TestKoGenStyles TESTNAME libs-komain-TestKoGenStyles ${TestKoGenStyles_SRCS})
target_link_libraries(TestKoGenStyles  ${KDE4_KDECORE_LIBS} komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kooasissettingstest_SRCS kooasissettingstest.cpp )
kde4_add_unit_test(kooasissettingstest TESTNAME libs-komain-kooasissettingstest ${kooasissettingstest_SRCS})
target_link_libraries(kooasissettingstest  ${KDE4_KDECORE_LIBS} komain )

########### end ###############

