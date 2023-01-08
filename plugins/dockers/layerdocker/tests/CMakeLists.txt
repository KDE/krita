include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

macro_add_unittest_definitions()

krita_add_broken_unit_tests(
    TestNodeView.cpp
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-dockers-layerdocker-"
    )
