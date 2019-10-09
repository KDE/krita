set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

macro_add_unittest_definitions()

########### next target ###############

ecm_add_test(KisMagneticWorkerTest.cc
    NAME_PREFIX plugins-magneticselection-
    LINK_LIBRARIES kritaselectiontools kritaimage Qt5::Test)
