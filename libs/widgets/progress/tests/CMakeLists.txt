set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/libs/widgets ${CMAKE_SOURCE_DIR}/libs/widgets/progress ${KDE4_INCLUDES}) 

set(KoProgressUpdater_test_SRCS KoProgressUpdater_test.cpp )
kde4_add_unit_test(KoProgressUpdaterTest TESTNAME libs-koprogress-KoProgressUpdaterTest ${KoProgressUpdater_test_SRCS})
target_link_libraries(KoProgressUpdaterTest  ${KDE4_KDEUI_LIBS} ${KDE4_THREADWEAVER_LIBRARY} koprogress ${QT_QTTEST_LIBRARY})
