macro_add_unittest_definitions()

include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../colorspaces)

add_definitions(-DFILES_DEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/data/")

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

include(KritaAddBrokenUnitTest)

add_executable(CCSGraph CCSGraph.cpp)
target_link_libraries(CCSGraph  kritapigment KF5::I18n)
ecm_mark_as_test(CCSGraph)

if (APPLE)
    ecm_add_tests(
        TestKoColorSet.cpp
        TestKoColorSpaceAbstract.cpp
        TestKoIntegerMaths.cpp
        TestConvolutionOpImpl.cpp
        TestKoChannelInfo.cpp
        NAME_PREFIX "libs-pigment-"
        LINK_LIBRARIES kritapigment KF5::I18n Qt5::Test
        TARGET_NAMES_VAR OK_TESTS
        ${MACOS_GUI_TEST})

    macos_test_fixrpath(${OK_TESTS})

    ecm_add_tests(
        TestColorConversion.cpp
        TestKoColorSpaceMaths.cpp

        NAME_PREFIX "libs-pigment-"
        LINK_LIBRARIES kritapigment Qt5::Test
        TARGET_NAMES_VAR OK_TESTS
        ${MACOS_GUI_TEST})

   macos_test_fixrpath(${OK_TESTS} CCSGraph)

    # Broken unit test.app can be run if installed
    krita_add_broken_unit_tests(
        TestKoColorSpaceRegistry.cpp
        TestColorConversionSystem.cpp
        TestKoColor.cpp
        KoRgbU8ColorSpaceTester.cpp
        TestKoColorSpaceSanity.cpp
        TestFallBackColorTransformation.cpp

        NAME_PREFIX "libs-pigment-"
        LINK_LIBRARIES kritapigment KF5::I18n Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST})

    macos_test_fixrpath(${BROKEN_TESTS})

    krita_add_broken_unit_tests(
        TestKisSwatchGroup.cpp
        TestKoStopGradient.cpp

        NAME_PREFIX "libs-pigment-"
        LINK_LIBRARIES kritapigment Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST})


    macos_test_fixrpath(${BROKEN_TESTS})


else (APPLE)

    ecm_add_tests(
        TestKoColorSet.cpp
        TestKoColorSpaceRegistry.cpp
        TestKoColorSpaceAbstract.cpp
        TestColorConversionSystem.cpp
        TestKoColor.cpp
        TestKoIntegerMaths.cpp
        TestConvolutionOpImpl.cpp
        KoRgbU8ColorSpaceTester.cpp
        TestKoColorSpaceSanity.cpp
        TestFallBackColorTransformation.cpp
        TestKoChannelInfo.cpp
        NAME_PREFIX "libs-pigment-"
        LINK_LIBRARIES kritapigment KF5::I18n Qt5::Test)


    ecm_add_tests(
        TestColorConversion.cpp
        TestKoColorSpaceMaths.cpp
        TestKisSwatchGroup.cpp
        TestKoStopGradient.cpp

        NAME_PREFIX "libs-pigment-"
        LINK_LIBRARIES kritapigment Qt5::Test)

endif()
