set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOODF_INCLUDES} )

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
kde4_add_unit_test(TestKoGenStyles TESTNAME libs-odf-TestKoGenStyles ${TestKoGenStyles_SRCS})
target_link_libraries(TestKoGenStyles  ${KDE4_KDECORE_LIBS} koodf ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kooasissettingstest_SRCS kooasissettingstest.cpp )
kde4_add_unit_test(kooasissettingstest TESTNAME libs-odf-kooasissettingstest ${kooasissettingstest_SRCS})
target_link_libraries(kooasissettingstest  ${KDE4_KDECORE_LIBS} koodf )

########### next target ###############

set(kooasisstoretest_SRCS kooasisstoretest.cpp )
kde4_add_unit_test(kooasisstoretest TESTNAME libs-odf-kooasisstoretest ${kooasisstoretest_SRCS})
target_link_libraries(kooasisstoretest  ${KDE4_KDECORE_LIBS} koodf ${QT_QT3SUPPORT_LIBRARY})

########### next target ###############

set(TestKoOdfLoadingContext_SRCS TestKoOdfLoadingContext.cpp )
kde4_add_unit_test(TestKoOdfLoadingContext TESTNAME libs-odf-TestKoOdfLoadingContext ${TestKoOdfLoadingContext_SRCS})
target_link_libraries(TestKoOdfLoadingContext  ${KDE4_KDECORE_LIBS} koodf ${QT_QTTEST_LIBRARY})

########### end ###############

