add_executable(CCSGraph CCSGraph.cpp)
target_link_libraries(CCSGraph kritapigment KF${KF_MAJOR}::I18n)
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

    LINK_LIBRARIES kritapigment KF${KF_MAJOR}::I18n kritatestsdk
    )

kis_add_test(
    TestCompositeOpInversion.cpp TestCompositeOpInversion_p.cpp
    TEST_NAME TestCompositeOpInversion
    LINK_LIBRARIES kritapigment KF${KF_MAJOR}::I18n kritatestsdk
    )

kis_add_tests(
    TestColorConversion.cpp
    TestKoColorSpaceMaths.cpp
    TestKisSwatchGroup.cpp
    TestKoStopGradient.cpp

    LINK_LIBRARIES kritapigment kritatestsdk
    )


macos_test_fixrpath(CCSGraph)

