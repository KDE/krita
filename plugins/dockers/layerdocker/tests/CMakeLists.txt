include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_SOURCE_DIR}/libs/image/metadata
                    ${CMAKE_SOURCE_DIR}/sdk/tests )
include(ECMAddTests)

macro_add_unittest_definitions()

include(KritaAddBrokenUnitTest)

krita_add_broken_unit_tests(
    TestNodeView.cpp
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-dockers-layerdocker-"
    )
