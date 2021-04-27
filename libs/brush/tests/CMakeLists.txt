########### next target ###############

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

if(APPLE)
    ecm_add_tests(
        TestAbrStorage.cpp
        NAME_PREFIX "libs-brush-"
        LINK_LIBRARIES kritaimage kritalibbrush Qt5::Test
        TARGET_NAMES_VAR OK_TESTS
        GUI
    )

    # Broken unit test.app can be run if installed
    krita_add_broken_unit_tests(
        kis_auto_brush_test.cpp
        kis_auto_brush_factory_test.cpp
        kis_gbr_brush_test.cpp
        kis_boundary_test.cpp
        kis_imagepipe_brush_test.cpp
        NAME_PREFIX "libs-brush-"
        LINK_LIBRARIES kritaimage kritalibbrush Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        GUI
    )

    macos_test_fixrpath(${OK_TESTS} ${BROKEN_TESTS})


else(APPLE)
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
endif()
