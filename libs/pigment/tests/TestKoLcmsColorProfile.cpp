
#include "TestKoLcmsColorProfile.h"

#include <cmath>

#include <kdebug.h>

#include "../colorprofiles/KoLcmsRGBColorProfile.h"

double koRound(double value)
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

    QCOMPARE(koRound(chromaticities.primaries.Red.x), koRound(profileRed.x));
    QCOMPARE(koRound(chromaticities.primaries.Red.y), koRound(profileRed.y));
    QCOMPARE(koRound(chromaticities.primaries.Red.Y), koRound(profileRed.Y));
    QCOMPARE(koRound(chromaticities.primaries.Green.x), koRound(profileGreen.x));
    QCOMPARE(koRound(chromaticities.primaries.Green.y), koRound(profileGreen.y));
    QCOMPARE(koRound(chromaticities.primaries.Green.Y), koRound(profileGreen.Y));
    QCOMPARE(koRound(chromaticities.primaries.Blue.x), koRound(profileBlue.x));
    QCOMPARE(koRound(chromaticities.primaries.Blue.y), koRound(profileBlue.y));
    QCOMPARE(koRound(chromaticities.primaries.Blue.Y), koRound(profileBlue.Y));
    QCOMPARE(koRound(chromaticities.whitePoint.x), koRound(profileWhite.x));
    QCOMPARE(koRound(chromaticities.whitePoint.y), koRound(profileWhite.y));

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

    QCOMPARE(koRound(profileChromaticities.primaries.Red.x),   koRound(chromaticities.primaries.Red.x));
    QCOMPARE(koRound(profileChromaticities.primaries.Red.y),   koRound(chromaticities.primaries.Red.y));
    QCOMPARE(koRound(profileChromaticities.primaries.Green.x), koRound(chromaticities.primaries.Green.x));
    QCOMPARE(koRound(profileChromaticities.primaries.Green.y), koRound(chromaticities.primaries.Green.y));
    QCOMPARE(koRound(profileChromaticities.primaries.Blue.x),  koRound(chromaticities.primaries.Blue.x));
    QCOMPARE(koRound(profileChromaticities.primaries.Blue.y),  koRound(chromaticities.primaries.Blue.y));
    QCOMPARE(koRound(profileChromaticities.whitePoint.x),      koRound(chromaticities.whitePoint.x));
    QCOMPARE(koRound(profileChromaticities.whitePoint.y),      koRound(chromaticities.whitePoint.y));

    cmsHPROFILE lcmsProfile = profile->lcmsProfile();

    LPGAMMATABLE redGamma = cmsReadICCGamma(lcmsProfile, icSigRedTRCTag);
    LPGAMMATABLE greenGamma = cmsReadICCGamma(lcmsProfile, icSigGreenTRCTag);
    LPGAMMATABLE blueGamma = cmsReadICCGamma(lcmsProfile, icSigBlueTRCTag);

    QCOMPARE(koRound(cmsEstimateGamma(redGamma)), gamma);
    QCOMPARE(koRound(cmsEstimateGamma(greenGamma)), gamma);
    QCOMPARE(koRound(cmsEstimateGamma(blueGamma)), gamma);
}

QTEST_MAIN(TestKoLcmsColorProfile)
#include "TestKoLcmsColorProfile.moc"

