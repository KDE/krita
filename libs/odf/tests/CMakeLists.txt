set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOODF_INCLUDES} )

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
kde4_add_unit_test(TestKoGenStyles TESTNAME libs-odf-TestKoGenStyles ${TestKoGenStyles_SRCS})
target_link_libraries(TestKoGenStyles koodf ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOdfSettings_SRCS TestOdfSettings.cpp )
kde4_add_unit_test(TestOdfSettings TESTNAME libs-odf-TestOdfSettings ${TestOdfSettings_SRCS})
target_link_libraries(TestOdfSettings koodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOdfStore_SRCS TestOdfStore.cpp )
kde4_add_unit_test(TestOdfStore TESTNAME libs-odf-TestOdfStore ${TestOdfStore_SRCS})
target_link_libraries(TestOdfStore koodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoOdfLoadingContext_SRCS TestKoOdfLoadingContext.cpp )
kde4_add_unit_test(TestKoOdfLoadingContext TESTNAME libs-odf-TestKoOdfLoadingContext ${TestKoOdfLoadingContext_SRCS})
target_link_libraries(TestKoOdfLoadingContext koodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### end ###############

