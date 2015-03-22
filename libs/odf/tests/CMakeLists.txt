set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOODF_INCLUDES} )

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
kde4_add_unit_test(TestKoGenStyles TESTNAME libs-odf-TestKoGenStyles ${TestKoGenStyles_SRCS})
target_link_libraries(TestKoGenStyles koodf Qt5::Test)

########### next target ###############

set(TestOdfSettings_SRCS TestOdfSettings.cpp )
kde4_add_unit_test(TestOdfSettings TESTNAME libs-odf-TestOdfSettings ${TestOdfSettings_SRCS})
target_link_libraries(TestOdfSettings koodf Qt5::Test)

########### next target ###############

set(TestKoOdfLoadingContext_SRCS TestKoOdfLoadingContext.cpp )
kde4_add_unit_test(TestKoOdfLoadingContext TESTNAME libs-odf-TestKoOdfLoadingContext ${TestKoOdfLoadingContext_SRCS})
target_link_libraries(TestKoOdfLoadingContext koodf Qt5::Test)

########### next target ###############

set(xmlwritertest_SRCS TestXmlWriter.cpp )

kde4_add_unit_test(TestXmlWriter TESTNAME libs-odf-TestXmlWriter ${xmlwritertest_SRCS})

target_link_libraries(TestXmlWriter koodf Qt5::Test)

########### next target ###############

set(xmlreadertest_SRCS TestXmlReader.cpp )
kde4_add_unit_test(TestXmlReader TESTNAME libs-odf-TestXmlReader ${xmlreadertest_SRCS})
target_link_libraries(TestXmlReader koodf Qt5::Test)

########### next target ###############

set(xmlreadertestwithoutspaces_SRCS TestXmlReaderWithoutSpaces.cpp )
kde4_add_unit_test(TestXmlReaderWithoutSpaces TESTNAME libs-odf-TestXmlReaderWithoutSpaces ${xmlreadertestwithoutspaces_SRCS})
target_link_libraries(TestXmlReaderWithoutSpaces koodf Qt5::Test)

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_add_executable(storedroptest TEST ${storedroptest_SRCS})

target_link_libraries(storedroptest koodf)

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_add_unit_test(kodomtest TESTNAME libs-store-kodomtest ${kodomtest_SRCS})

target_link_libraries(kodomtest koodf Qt5::Test )

########### next target ###############

set(storageTest_SRCS TestStorage.cpp )
kde4_add_unit_test(TestStorage TESTNAME libs-store-storage ${storageTest_SRCS})
target_link_libraries(TestStorage koodf Qt5::Test)

########### next target ###############

set(TestKoUnit_SRCS TestKoUnit.cpp)
kde4_add_unit_test(TestKoUnit TESTNAME libs-koodf-TestKoUnit ${TestKoUnit_SRCS})
target_link_libraries(TestKoUnit koodf Qt5::Test)

########### next target ###############

set(TestNumberStyle_SRCS TestNumberStyle.cpp )
kde4_add_unit_test(TestNumberStyle TESTNAME libs-odf-TestNumberStyle ${TestNumberStyle_SRCS})
target_link_libraries(TestNumberStyle koodf Qt5::Test)

########### next target ###############

set(TestKoElementReference_SRCS TestKoElementReference.cpp)
kde4_add_unit_test(TestKoElementReference TESTNAME libs-koodf-TestKoElementReference ${TestKoElementReference_SRCS})
target_link_libraries(TestKoElementReference koodf Qt5::Test)

########### end ###############
