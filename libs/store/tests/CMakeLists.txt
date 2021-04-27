macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include(ECMAddTests)

ecm_add_test(
    ../KoLZF.cpp TestKoLZF.cpp
    TEST_NAME TestKoLZF
    LINK_LIBRARIES kritastore Qt5::Test
    NAME_PREFIX "libs-odf"
    ${MACOS_GUI_TEST})

ecm_add_test(
    ../KoLZF.cpp TestKoXmlVector.cpp
    TEST_NAME TestKoXmlVector
    LINK_LIBRARIES kritastore Qt5::Test
    NAME_PREFIX "libs-odf"
    ${MACOS_GUI_TEST})


########### manual test for file contents ###############
add_executable(storedroptest storedroptest.cpp)
target_link_libraries(storedroptest kritastore Qt5::Widgets)
ecm_mark_as_test(storedroptest)

macos_test_fixrpath(
    TestKoLZF
    TestKoXmlVector
    storedroptest
    )
