set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOODF_INCLUDES} )

########### next target ###############

set(lzftest_SRCS ../KoLZF.cpp TestKoLZF.cpp )
kde4_add_unit_test(TestKoLZF TESTNAME libs-odf-TestKoLZF ${lzftest_SRCS})
target_link_libraries(TestKoLZF Qt5::Test)

########### next target ###############

set(xmlvectortest_SRCS ../KoLZF.cpp TestKoXmlVector.cpp )
kde4_add_unit_test(TestKoXmlVector TESTNAME libs-odf-TestKoXmlVector ${xmlvectortest_SRCS})
target_link_libraries(TestKoXmlVector Qt5::Test)

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )
kde4_add_executable(storedroptest TEST ${storedroptest_SRCS})
target_link_libraries(storedroptest kritastore Qt5::Widgets)


