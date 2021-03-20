macro_add_unittest_definitions()

include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../colorspaces)

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

ecm_add_tests(
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
    # TestKoColorSet.cpp
    TestKoStopGradient.cpp

    NAME_PREFIX "libs-pigment-"
    LINK_LIBRARIES kritapigment Qt5::Test)

add_executable(CCSGraph CCSGraph.cpp)
target_link_libraries(CCSGraph  kritapigment KF5::I18n)
ecm_mark_as_test(CCSGraph)
