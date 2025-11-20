########### next target ###############

if (HAVE_OCIO_V2)
  set(KRITA_LUTDOCKER_SOURCES
    ../ocio_display_filter_vfx2021.cpp
  )
else()
  set(KRITA_LUTDOCKER_SOURCES
    ../ocio_display_filter_vfx2020.cpp
  )
endif()

kis_add_test(
    kis_ocio_display_filter_test.cpp
    ../black_white_point_chooser.cpp  
    ${KRITA_LUTDOCKER_SOURCES}
     $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME KisOcioDisplayFilterTest
    LINK_LIBRARIES kritaui OpenColorIO::OpenColorIO KF${KF_MAJOR}::I18n kritatestsdk kritalutdocker_static
    NAME_PREFIX "plugins-dockers-lut-"
    )

