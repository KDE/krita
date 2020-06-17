########### next target ###############
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/sdk/tests )
include_directories(${LIBMYPAINT_INCLUDE_DIR})
link_directories(${LIBMYPAINT_LIBRARIES})

macro_add_unittest_definitions()

include(KritaAddBrokenUnitTest)

ecm_add_test(kis_mypaintop_test.cpp ../kis_my_paintop_settings.cpp ../kis_mypaint_brush.cpp ../kis_mypaint_surface.cpp
    TEST_NAME KisMyPaintOpTest
    NAME_PREFIX plugins-kismypaintop-
    LINK_LIBRARIES kritaimage kritamypaintop kritalibpaintop mypaint Qt5::Test)

#krita_add_broken_unit_test(kis_embedded_pattern_manager_test.cpp
#    NAME_PREFIX plugins-libpaintop-
#    LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)

