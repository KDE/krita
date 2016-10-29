macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include_directories(SYSTEM
    ${OCIO_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )

########### next target ###############

krita_add_broken_unit_test(kis_ocio_display_filter_test.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME krita-ocio-KisOcioDisplayFilterTest
    LINK_LIBRARIES kritaimage kritaui kritalutdocker ${OCIO_LIBRARIES} KF5::I18n Qt5::Test)
