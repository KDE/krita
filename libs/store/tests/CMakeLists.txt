set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
########### next target ###############

set(storage_test2_SRCS storage_test2.cpp )

kde4_add_unit_test(storage_test2 TESTNAME libs-store-storage_test2 ${storage_test2_SRCS})

target_link_libraries(storage_test2 kostore )

########### next target ###############

set(xmlwritertest_SRCS TestXmlWriter.cpp )

kde4_add_unit_test(TestXmlWriter TESTNAME libs-store-testxmlwriter ${xmlwritertest_SRCS})

target_link_libraries(TestXmlWriter kostore ${QT_QTTEST_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

########### next target ###############

set(koxmlreader_SRCS koxmlreadertest.cpp )

kde4_add_unit_test(koxmlreadertest TESTNAME libs-store-koxmlreadertest ${koxmlreader_SRCS})

target_link_libraries(koxmlreadertest kostore ${QT_QTTEST_LIBRARY} )

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_add_executable(storedroptest TEST ${storedroptest_SRCS})

target_link_libraries(storedroptest kostore ${QT_QT3SUPPORT_LIBRARY})

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_add_unit_test(kodomtest TESTNAME libs-store-kodomtest ${kodomtest_SRCS})

target_link_libraries(kodomtest kostore ${QT_QTTEST_LIBRARY} )

########### next target ###############

set(storageTest_SRCS TestStorage.cpp )

kde4_add_unit_test(TestStorage TESTNAME libs-store-storage ${storageTest_SRCS})

target_link_libraries(TestStorage kostore ${QT_QTTEST_LIBRARY} )

########### end ###############
