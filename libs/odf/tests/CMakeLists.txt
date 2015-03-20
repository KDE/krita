set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOODF_INCLUDES} )

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
add_executable(TestKoGenStylesNAME libs-odf-TestKoGenStyles ${TestKoGenStyles_SRCS})
add_test(TestKoGenStyles TestKoGenStyles)
ecm_mark_as_test(TestKoGenStyles)
target_link_libraries(TestKoGenStyles koodf Qt5::Test)

########### next target ###############

set(TestOdfSettings_SRCS TestOdfSettings.cpp )
add_executable(TestOdfSettingsNAME libs-odf-TestOdfSettings ${TestOdfSettings_SRCS})
add_test(TestOdfSettings TestOdfSettings)
ecm_mark_as_test(TestOdfSettings)
target_link_libraries(TestOdfSettings koodf Qt5::Gui Qt5::Test)

########### next target ###############

set(TestKoOdfLoadingContext_SRCS TestKoOdfLoadingContext.cpp )
add_executable(TestKoOdfLoadingContextNAME libs-odf-TestKoOdfLoadingContext ${TestKoOdfLoadingContext_SRCS})
add_test(TestKoOdfLoadingContext TestKoOdfLoadingContext)
ecm_mark_as_test(TestKoOdfLoadingContext)
target_link_libraries(TestKoOdfLoadingContext koodf Qt5::Gui Qt5::Test)

########### next target ###############

set(xmlwritertest_SRCS TestXmlWriter.cpp )

add_executable(TestXmlWriterNAME libs-odf-TestXmlWriter ${xmlwritertest_SRCS})
add_test(TestXmlWriter TestXmlWriter)
ecm_mark_as_test(TestXmlWriter)

target_link_libraries(TestXmlWriter koodf Qt5::Test  )

########### next target ###############

set(xmlreadertest_SRCS TestXmlReader.cpp )
add_executable(TestXmlReaderNAME libs-odf-TestXmlReader ${xmlreadertest_SRCS})
add_test(TestXmlReader TestXmlReader)
ecm_mark_as_test(TestXmlReader)
target_link_libraries(TestXmlReader koodf Qt5::Test  )

########### next target ###############

set(xmlreadertestwithoutspaces_SRCS TestXmlReaderWithoutSpaces.cpp )
add_executable(TestXmlReaderWithoutSpacesNAME libs-odf-TestXmlReaderWithoutSpaces ${xmlreadertestwithoutspaces_SRCS})
add_test(TestXmlReaderWithoutSpaces TestXmlReaderWithoutSpaces)
ecm_mark_as_test(TestXmlReaderWithoutSpaces)
target_link_libraries(TestXmlReaderWithoutSpaces koodf Qt5::Test  )

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

add_executable(storedroptest TEST ${storedroptest_SRCS})

target_link_libraries(storedroptest koodf  Qt5::Gui )

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

add_executable(kodomtestNAME libs-store-kodomtest ${kodomtest_SRCS})
add_test(kodomtest kodomtest)
ecm_mark_as_test(kodomtest)

target_link_libraries(kodomtest koodf Qt5::Test Qt5::Gui  )

########### next target ###############

set(storageTest_SRCS TestStorage.cpp )
add_executable(TestStorageNAME libs-store-storage ${storageTest_SRCS})
add_test(TestStorage TestStorage)
ecm_mark_as_test(TestStorage)
target_link_libraries(TestStorage koodf Qt5::Test Qt5::Gui )

########### next target ###############

set(TestKoUnit_SRCS TestKoUnit.cpp)
add_executable(TestKoUnitNAME libs-koodf-TestKoUnit ${TestKoUnit_SRCS})
add_test(TestKoUnit TestKoUnit)
ecm_mark_as_test(TestKoUnit)
target_link_libraries(TestKoUnit koodf Qt5::Test)

########### next target ###############

set(TestNumberStyle_SRCS TestNumberStyle.cpp )
add_executable(TestNumberStyleNAME libs-odf-TestNumberStyle ${TestNumberStyle_SRCS})
add_test(TestNumberStyle TestNumberStyle)
ecm_mark_as_test(TestNumberStyle)
target_link_libraries(TestNumberStyle koodf Qt5::Gui Qt5::Test)

########### next target ###############

set(TestKoElementReference_SRCS TestKoElementReference.cpp)
add_executable(TestKoElementReferenceNAME libs-koodf-TestKoElementReference ${TestKoElementReference_SRCS})
add_test(TestKoElementReference TestKoElementReference)
ecm_mark_as_test(TestKoElementReference)
target_link_libraries(TestKoElementReference koodf Qt5::Test)

########### end ###############
