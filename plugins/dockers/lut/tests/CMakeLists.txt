macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests ../)
include_directories(SYSTEM
    ${OCIO_INCLUDE_DIR}
    )

########### next target ###############

krita_add_broken_unit_test(kis_ocio_display_filter_test.cpp 
    ../black_white_point_chooser.cpp  
    ../ocio_display_filter.cpp  
    ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME KisOcioDisplayFilterTest
    LINK_LIBRARIES kritaui ${OCIO_LIBRARIES} KF5::I18n Qt5::Test
    NAME_PREFIX "plugins-dockers-lut-")
