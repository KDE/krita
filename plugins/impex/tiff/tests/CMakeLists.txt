if(LCMS2_FOUND)
  add_definitions( -DHAVE_LCMS2 )
endif()

include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

include(ECMAddTests)
include(KritaAddBrokenUnitTest)
macro_add_unittest_definitions()

ecm_add_tests(
    kis_tiff_test.cpp
    NAME_PREFIX "krita-plugin-impex-tiff-"
    LINK_LIBRARIES kritaui Qt5::Test
)
