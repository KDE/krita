
#include "TestKoLcmsColorProfile.h"

#include <cmath>

qreal testRounding(qreal value)
{
    qreal factor;
    int temp;
    const int numPlaces = 3;

    factor = pow(10.0, numPlaces);
    temp = (int)(value * factor + 0.5);
    return temp / factor;
}

void TestKoLcmsColorProfile::testChromaticitiesFromProfile()
{
#if 0
    cmsHPROFILE profile = cmsCreate_sRGBProfile();

    KoLcmsRGBColorProfile::Chromaticities chromaticities = KoLcmsRGBColorProfile::chromaticitiesFromProfile(profile);

    const cmsCIExyY profileRed =   {0.6400f, 0.3300f, 0.212656f};
    const cmsCIExyY profileGreen = {0.3000f, 0.6000f, 0.715158f};
    const cmsCIExyY profileBlue =  {0.1500f, 0.0600f, 0.072186f};
    const cmsCIExyY profileWhite = {0.3127f, 0.3290f, 1.000000f};

    QCOMPARE(testRounding(chromaticities.primaries.Red.x), testRounding(profileRed.x));
    QCOMPARE(testRounding(chromaticities.primaries.Red.y), testRounding(profileRed.y));
    QCOMPARE(testRounding(chromaticities.primaries.Red.Y), testRounding(profileRed.Y));
    QCOMPARE(testRounding(chromaticities.primaries.Green.x), testRounding(profileGreen.x));
    QCOMPARE(testRounding(chromaticities.primaries.Green.y), testRounding(profileGreen.y));
    QCOMPARE(testRounding(chromaticities.primaries.Green.Y), testRounding(profileGreen.Y));
    QCOMPARE(testRounding(chromaticities.primaries.Blue.x), testRounding(profileBlue.x));
    QCOMPARE(testRounding(chromaticities.primaries.Blue.y), testRounding(profileBlue.y));
    QCOMPARE(testRounding(chromaticities.primaries.Blue.Y), testRounding(profileBlue.Y));
    QCOMPARE(testRounding(chromaticities.whitePoint.x), testRounding(profileWhite.x));
    QCOMPARE(testRounding(chromaticities.whitePoint.y), testRounding(profileWhite.y));

    cmsCloseProfile(profile);
#endif
}

void TestKoLcmsColorProfile::testProfileCreationFromChromaticities()
{
#if 0
    KoLcmsRGBColorProfile::Chromaticities chromaticities;

    chromaticities.primaries.Red.x = 0.7347f;
    chromaticities.primaries.Red.y = 0.2653f;
    chromaticities.primaries.Red.Y = 1.0f;
    chromaticities.primaries.Green.x = 0.1596f;
    chromaticities.primaries.Green.y = 0.8404f;
    chromaticities.primaries.Green.Y = 1.0f;
    chromaticities.primaries.Blue.x = 0.0366f;
    chromaticities.primaries.Blue.y = 0.0001f;
    chromaticities.primaries.Blue.Y = 1.0f;
    chromaticities.whitePoint.x = 0.34567f;
    chromaticities.whitePoint.y = 0.35850f;
    chromaticities.whitePoint.Y = 1.0f;

    qreal gamma = 1.75f;

    KoLcmsRGBColorProfile *profile = new KoLcmsRGBColorProfile(chromaticities, gamma);
    QVERIFY(profile != 0);

    QCOMPARE(profile->colorSpaceSignature(), icSigRgbData);

    cmsHPROFILE lcmsProfile = profile->lcmsProfile();

    KoLcmsRGBColorProfile::Chromaticities profileChromaticities =
        KoLcmsRGBColorProfile::chromaticitiesFromProfile(lcmsProfile);

    QCOMPARE(testRounding(profileChromaticities.primaries.Red.x),   testRounding(chromaticities.primaries.Red.x));
    QCOMPARE(testRounding(profileChromaticities.primaries.Red.y),   testRounding(chromaticities.primaries.Red.y));
    QCOMPARE(testRounding(profileChromaticities.primaries.Green.x), testRounding(chromaticities.primaries.Green.x));
    QCOMPARE(testRounding(profileChromaticities.primaries.Green.y), testRounding(chromaticities.primaries.Green.y));
    QCOMPARE(testRounding(profileChromaticities.primaries.Blue.x),  testRounding(chromaticities.primaries.Blue.x));
    QCOMPARE(testRounding(profileChromaticities.primaries.Blue.y),  testRounding(chromaticities.primaries.Blue.y));
    QCOMPARE(testRounding(profileChromaticities.whitePoint.x),      testRounding(chromaticities.whitePoint.x));
    QCOMPARE(testRounding(profileChromaticities.whitePoint.y),      testRounding(chromaticities.whitePoint.y));

    LPGAMMATABLE redGamma = cmsReadICCGamma(lcmsProfile, icSigRedTRCTag);
    LPGAMMATABLE greenGamma = cmsReadICCGamma(lcmsProfile, icSigGreenTRCTag);
    LPGAMMATABLE blueGamma = cmsReadICCGamma(lcmsProfile, icSigBlueTRCTag);

    QCOMPARE(testRounding(cmsEstimateGamma(redGamma)), gamma);
    QCOMPARE(testRounding(cmsEstimateGamma(greenGamma)), gamma);
    QCOMPARE(testRounding(cmsEstimateGamma(blueGamma)), gamma);

    QString expectedProfileName = QString("lcms virtual RGB profile - R(%1, %2) G(%3, %4) B(%5, %6) W(%7, %8) gamma %9")
                                  .arg(chromaticities.primaries.Red.x)
                                  .arg(chromaticities.primaries.Red.y)
                                  .arg(chromaticities.primaries.Green.x)
                                  .arg(chromaticities.primaries.Green.y)
                                  .arg(chromaticities.primaries.Blue.x)
                                  .arg(chromaticities.primaries.Blue.y)
                                  .arg(chromaticities.whitePoint.x)
                                  .arg(chromaticities.whitePoint.y)
                                  .arg(gamma);

    QCOMPARE(profile->name(), expectedProfileName);
    QCOMPARE(QString(cmsTakeProductDesc(lcmsProfile)), expectedProfileName);

    profileChromaticities = profile->chromaticities();

    QCOMPARE(profileChromaticities.primaries.Red.x,   chromaticities.primaries.Red.x);
    QCOMPARE(profileChromaticities.primaries.Red.y,   chromaticities.primaries.Red.y);
    QCOMPARE(profileChromaticities.primaries.Green.x, chromaticities.primaries.Green.x);
    QCOMPARE(profileChromaticities.primaries.Green.y, chromaticities.primaries.Green.y);
    QCOMPARE(profileChromaticities.primaries.Blue.x,  chromaticities.primaries.Blue.x);
    QCOMPARE(profileChromaticities.primaries.Blue.y,  chromaticities.primaries.Blue.y);
    QCOMPARE(profileChromaticities.whitePoint.x,      chromaticities.whitePoint.x);
    QCOMPARE(profileChromaticities.whitePoint.y,      chromaticities.whitePoint.y);

    const QString testProfileName = "Test Profile Name";

    profile = new KoLcmsRGBColorProfile(chromaticities, gamma, testProfileName);
    lcmsProfile = profile->lcmsProfile();

    QCOMPARE(profile->name(), testProfileName);
    QCOMPARE(QString(cmsTakeProductDesc(lcmsProfile)), testProfileName);
#endif
}

QTEST_MAIN(TestKoLcmsColorProfile)
#include <TestKoLcmsColorProfile.moc>

