########### next target ###############

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_SOURCE_DIR}/libs/image/metadata
    ${CMAKE_SOURCE_DIR}/sdk/tests
)
include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)

if(HAVE_VC)
  include_directories(SYSTEM
      ${Vc_INCLUDE_DIR}
  )
endif()

macro_add_unittest_definitions()

include(ECMAddTests)

ecm_add_tests(
    kis_auto_brush_test.cpp
    kis_auto_brush_factory_test.cpp
    kis_gbr_brush_test.cpp
    kis_boundary_test.cpp
    kis_imagepipe_brush_test.cpp
    TestAbrStorage.cpp
    NAME_PREFIX "libs-brush-"
    LINK_LIBRARIES kritaimage kritalibbrush Qt5::Test
)
