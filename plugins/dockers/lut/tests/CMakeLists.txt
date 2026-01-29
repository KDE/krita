########### next target ###############

kis_add_test(
    kis_ocio_display_filter_test.cpp
    ../black_white_point_chooser.cpp  
    ../ocio_display_filter_vfx2021.cpp
     $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME KisOcioDisplayFilterTest
    LINK_LIBRARIES kritaui OpenColorIO::OpenColorIO KF${KF_MAJOR}::I18n kritatestsdk kritalutdocker_static
    )

