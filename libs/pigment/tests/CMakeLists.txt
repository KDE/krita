include(KritaAddBrokenUnitTest)

add_executable(CCSGraph CCSGraph.cpp)
target_link_libraries(CCSGraph kritapigment KF5::I18n)
ecm_mark_as_test(CCSGraph)


kis_add_tests(
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
    TestCompositeOpInversion.cpp
    NAME_PREFIX "libs-pigment-"
    LINK_LIBRARIES kritapigment KF5::I18n kritatestsdk
    )


kis_add_tests(
    TestColorConversion.cpp
    TestKoColorSpaceMaths.cpp
    TestKisSwatchGroup.cpp
    TestKoStopGradient.cpp

    NAME_PREFIX "libs-pigment-"
    LINK_LIBRARIES kritapigment kritatestsdk
    )


macos_test_fixrpath(CCSGraph)

