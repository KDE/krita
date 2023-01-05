########### next target ###############

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/image/metadata
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

macro_add_unittest_definitions()

include(ECMAddTests)

kis_add_tests(
    kis_auto_brush_test.cpp
    kis_auto_brush_factory_test.cpp
    kis_gbr_brush_test.cpp
    kis_png_brush_test.cpp
    kis_boundary_test.cpp
    kis_imagepipe_brush_test.cpp
    TestAbrStorage.cpp
    NAME_PREFIX "libs-brush-"
    LINK_LIBRARIES kritaimage kritalibbrush Qt5::Test
    )

