macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include_directories( ../colorspaces/cmyk_u16
    ../colorspaces/cmyk_u8
    ../colorspaces/gray_u16
    ../colorspaces/gray_u8
    ../colorspaces/lab_u16
    ../colorspaces/rgb_u16
    ../colorspaces/rgb_u8
    ../colorspaces/xyz_u16
    ../colorprofiles
    ..
)
if(OpenEXR_FOUND)
    include_directories(SYSTEM ${OPENEXR_INCLUDE_DIRS})
endif()
include_directories( ${LCMS2_INCLUDE_DIR} )


if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

if (APPLE)
    include(KritaAddBrokenUnitTest)

    krita_add_broken_unit_tests(
        TestKoLcmsColorProfile.cpp
        TestColorSpaceRegistry.cpp
        TestLcmsRGBP2020PQColorSpace.cpp
        TestProfileGeneration.cpp
        NAME_PREFIX "plugins-lcmsengine-"
        LINK_LIBRARIES kritawidgets kritapigment KF5::I18n Qt5::Test ${LCMS2_LIBRARIES}
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST}
        )

    macos_test_fixrpath(${BROKEN_TESTS})


else (APPLE)
    ecm_add_tests(
        TestKoLcmsColorProfile.cpp
        TestColorSpaceRegistry.cpp
        TestLcmsRGBP2020PQColorSpace.cpp
        TestProfileGeneration.cpp
        NAME_PREFIX "plugins-lcmsengine-"
        LINK_LIBRARIES kritawidgets kritapigment KF5::I18n Qt5::Test ${LCMS2_LIBRARIES})

endif()


