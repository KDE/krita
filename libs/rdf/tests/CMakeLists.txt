set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KORDF_INCLUDES} )

set(TestKoDocumentRdf_SRCS TestKoDocumentRdf.cpp )
kde4_add_unit_test(TestKoDocumentRdf TESTNAME libs-kordf-TestKoDocumentRdf ${TestKoDocumentRdf_SRCS})
target_link_libraries(TestKoDocumentRdf kordf Qt5::Test)

set(rdf_test_SRCS rdf_test.cpp )
kde4_add_unit_test(rdf_test TESTNAME libs-kordf-rdf_test ${rdf_test_SRCS})
target_link_libraries(rdf_test kordf Qt5::Test)
