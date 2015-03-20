set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KORDF_INCLUDES} )

set(TestKoDocumentRdf_SRCS TestKoDocumentRdf.cpp )
add_executable(TestKoDocumentRdfNAME libs-kordf-TestKoDocumentRdf ${TestKoDocumentRdf_SRCS})
add_test(TestKoDocumentRdf TestKoDocumentRdf)
ecm_mark_as_test(TestKoDocumentRdf)
target_link_libraries(TestKoDocumentRdf kordf Qt5::Test)

set(rdf_test_SRCS rdf_test.cpp )
add_executable(rdf_testNAME libs-kordf-rdf_test ${rdf_test_SRCS})
add_test(rdf_test rdf_test)
ecm_mark_as_test(rdf_test)
target_link_libraries(rdf_test kordf Qt5::Test)
