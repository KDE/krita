
#include "TestKoLcmsColorProfile.h"

#include <cmath>

#include <kdebug.h>

#include "../colorprofiles/KoLcmsRGBColorProfile.h"

double testRounding(double value)
{
    double factor;
    int temp;
    const int numPlaces = 3;

    factor = pow(10.0, numPlaces);
    temp = (int)(value * factor + 0.5);
    return temp / factor;
}

void TestKoLcmsColorProfile::testChromaticitiesFromProfile()
{
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
}

void TestKoLcmsColorProfile::testProfileCreationFromChromaticities()
{
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

    double gamma = 1.75f;

    KoLcmsRGBColorProfile *profile = new KoLcmsRGBColorProfile(chromaticities, gamma);
    QVERIFY(profile != 0);

    QCOMPARE(profile->colorSpaceSignature(), icSigRgbData);

    KoLcmsRGBColorProfile::Chromaticities profileChromaticities = 
        KoLcmsRGBColorProfile::chromaticitiesFromProfile(profile);

    QCOMPARE(testRounding(profileChromaticities.primaries.Red.x),   testRounding(chromaticities.primaries.Red.x));
    QCOMPARE(testRounding(profileChromaticities.primaries.Red.y),   testRounding(chromaticities.primaries.Red.y));
    QCOMPARE(testRounding(profileChromaticities.primaries.Green.x), testRounding(chromaticities.primaries.Green.x));
    QCOMPARE(testRounding(profileChromaticities.primaries.Green.y), testRounding(chromaticities.primaries.Green.y));
    QCOMPARE(testRounding(profileChromaticities.primaries.Blue.x),  testRounding(chromaticities.primaries.Blue.x));
    QCOMPARE(testRounding(profileChromaticities.primaries.Blue.y),  testRounding(chromaticities.primaries.Blue.y));
    QCOMPARE(testRounding(profileChromaticities.whitePoint.x),      testRounding(chromaticities.whitePoint.x));
    QCOMPARE(testRounding(profileChromaticities.whitePoint.y),      testRounding(chromaticities.whitePoint.y));

    cmsHPROFILE lcmsProfile = profile->lcmsProfile();

    LPGAMMATABLE redGamma = cmsReadICCGamma(lcmsProfile, icSigRedTRCTag);
    LPGAMMATABLE greenGamma = cmsReadICCGamma(lcmsProfile, icSigGreenTRCTag);
    LPGAMMATABLE blueGamma = cmsReadICCGamma(lcmsProfile, icSigBlueTRCTag);

    QCOMPARE(testRounding(cmsEstimateGamma(redGamma)), gamma);
    QCOMPARE(testRounding(cmsEstimateGamma(greenGamma)), gamma);
    QCOMPARE(testRounding(cmsEstimateGamma(blueGamma)), gamma);
}

QTEST_MAIN(TestKoLcmsColorProfile)
#include "TestKoLcmsColorProfile.moc"

