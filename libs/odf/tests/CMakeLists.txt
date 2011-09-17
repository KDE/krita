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

set(TestKoOdfLoadingContext_SRCS TestKoOdfLoadingContext.cpp )
kde4_add_unit_test(TestKoOdfLoadingContext TESTNAME libs-odf-TestKoOdfLoadingContext ${TestKoOdfLoadingContext_SRCS})
target_link_libraries(TestKoOdfLoadingContext koodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(xmlwritertest_SRCS TestXmlWriter.cpp )

kde4_add_unit_test(TestXmlWriter TESTNAME libs-store-testxmlwriter ${xmlwritertest_SRCS})

target_link_libraries(TestXmlWriter koodf ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(xmlreadertest_SRCS TestXmlReader.cpp )
kde4_add_unit_test(TestXmlReader TESTNAME libs-store-testxmlreader ${xmlreadertest_SRCS})
target_link_libraries(TestXmlReader koodf ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(xmlreadertestwithoutspaces_SRCS TestXmlReaderWithoutSpaces.cpp )
kde4_add_unit_test(TestXmlReaderWithoutSpaces TESTNAME libs-store-testxmlreader ${xmlreadertestwithoutspaces_SRCS})
target_link_libraries(TestXmlReaderWithoutSpaces koodf ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_add_executable(storedroptest TEST ${storedroptest_SRCS})

target_link_libraries(storedroptest koodf  ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_add_unit_test(kodomtest TESTNAME libs-store-kodomtest ${kodomtest_SRCS})

target_link_libraries(kodomtest koodf ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} )

########### next target ###############

set(storageTest_SRCS TestStorage.cpp )

kde4_add_unit_test(TestStorage TESTNAME libs-store-storage ${storageTest_SRCS})

target_link_libraries(TestStorage koodf ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kopropertiestest_SRCS KoPropertiesTest.cpp )
kde4_add_unit_test(KoPropertiesTest TESTNAME libs-kobase-KoPropertiesTest ${kopropertiestest_SRCS})
target_link_libraries(KoPropertiesTest  ${KDE4_KDEUI_LIBS} koodf ${QT_QTTEST_LIBRARY})

set(TestKoUnit_SRCS TestKoUnit.cpp)
kde4_add_unit_test(TestKoUnit TESTNAME libs-kobase-TestKoUnit ${TestKoUnit_SRCS})
target_link_libraries(TestKoUnit koodf ${QT_QTTEST_LIBRARY})

########### end ###############
