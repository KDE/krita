set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")

#add_subdirectory(filedialogtester)

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

ecm_add_tests(
    zoomhandler_test.cpp
    zoomcontroller_test.cpp
    KoResourceTaggingTest.cpp
    kis_parse_spin_boxes_test.cpp
    KoAnchorSelectionWidgetTest.cpp
    NAME_PREFIX "libs-widgets-"
    LINK_LIBRARIES kritawidgets Qt5::Test)
