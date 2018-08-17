set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)

ecm_add_test(
    ../KoLZF.cpp TestKoLZF.cpp
    TEST_NAME TestKoLZF
    LINK_LIBRARIES kritastore Qt5::Test
    NAME_PREFIX "libs-odf")

ecm_add_test(
    ../KoLZF.cpp TestKoXmlVector.cpp
    TEST_NAME TestKoXmlVector
    LINK_LIBRARIES kritastore Qt5::Test
    NAME_PREFIX "libs-odf")

########### manual test for file contents ###############

add_executable(storedroptest storedroptest.cpp)
target_link_libraries(storedroptest kritastore Qt5::Widgets)
ecm_mark_as_test(storedroptest)
