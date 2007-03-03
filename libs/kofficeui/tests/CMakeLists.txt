
if(KDE4_BUILD_TESTS)

include_directories(  ${KOFFICECORE_INCLUDES} ${KOFFICEUI_INCLUDES} )

########### next target ###############

set(zoomhandler_test_SRCS zoomhandler_test.cpp )

kde4_automoc(${zoomhandler_test_SRCS})

kde4_add_executable(zoomhandler_test RUN_UNINSTALLED ${zoomhandler_test_SRCS})

target_link_libraries(zoomhandler_test  ${KDE4_KDEUI_LIBS} kofficeui kofficecore ${QT_QTTEST_LIBRARY})

add_test(libs-kofficeui-zoomhandler_test zoomhandler_test)

########### end ###############

endif(KDE4_BUILD_TESTS)
