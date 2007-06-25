
#include "TestKoLcmsColorProfile.h"

#include <cmath>

#include <kdebug.h>

#include "../colorprofiles/KoLcmsRGBColorProfile.h"

double round(double value)
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

    QCOMPARE(round(chromaticities.primaries.Red.x), round(profileRed.x));
    QCOMPARE(round(chromaticities.primaries.Red.y), round(profileRed.y));
    QCOMPARE(round(chromaticities.primaries.Red.Y), round(profileRed.Y));
    QCOMPARE(round(chromaticities.primaries.Green.x), round(profileGreen.x));
    QCOMPARE(round(chromaticities.primaries.Green.y), round(profileGreen.y));
    QCOMPARE(round(chromaticities.primaries.Green.Y), round(profileGreen.Y));
    QCOMPARE(round(chromaticities.primaries.Blue.x), round(profileBlue.x));
    QCOMPARE(round(chromaticities.primaries.Blue.y), round(profileBlue.y));
    QCOMPARE(round(chromaticities.primaries.Blue.Y), round(profileBlue.Y));
    QCOMPARE(round(chromaticities.whitePoint.x), round(profileWhite.x));
    QCOMPARE(round(chromaticities.whitePoint.y), round(profileWhite.y));

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

    QCOMPARE(round(profileChromaticities.primaries.Red.x),   round(chromaticities.primaries.Red.x));
    QCOMPARE(round(profileChromaticities.primaries.Red.y),   round(chromaticities.primaries.Red.y));
    QCOMPARE(round(profileChromaticities.primaries.Green.x), round(chromaticities.primaries.Green.x));
    QCOMPARE(round(profileChromaticities.primaries.Green.y), round(chromaticities.primaries.Green.y));
    QCOMPARE(round(profileChromaticities.primaries.Blue.x),  round(chromaticities.primaries.Blue.x));
    QCOMPARE(round(profileChromaticities.primaries.Blue.y),  round(chromaticities.primaries.Blue.y));
    QCOMPARE(round(profileChromaticities.whitePoint.x),      round(chromaticities.whitePoint.x));
    QCOMPARE(round(profileChromaticities.whitePoint.y),      round(chromaticities.whitePoint.y));

    cmsHPROFILE lcmsProfile = profile->lcmsProfile();

    LPGAMMATABLE redGamma = cmsReadICCGamma(lcmsProfile, icSigRedTRCTag);
    LPGAMMATABLE greenGamma = cmsReadICCGamma(lcmsProfile, icSigGreenTRCTag);
    LPGAMMATABLE blueGamma = cmsReadICCGamma(lcmsProfile, icSigBlueTRCTag);

    QCOMPARE(round(cmsEstimateGamma(redGamma)), gamma);
    QCOMPARE(round(cmsEstimateGamma(greenGamma)), gamma);
    QCOMPARE(round(cmsEstimateGamma(blueGamma)), gamma);
}

QTEST_MAIN(TestKoLcmsColorProfile)
#include "TestKoLcmsColorProfile.moc"

