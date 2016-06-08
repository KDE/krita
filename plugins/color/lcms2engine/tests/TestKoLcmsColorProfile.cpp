
#include "TestKoLcmsColorProfile.h"
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <LcmsColorProfileContainer.h>

#include <KoColor.h>

#include <QTest>

#include <lcms2.h>
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

void TestKoLcmsColorProfile::testConversion()
{
    const KoColorSpace *sRgb = KoColorSpaceRegistry::instance()->rgb16("sRGB built-in");
    Q_ASSERT(sRgb);
    const KoColorSpace *linearRgb = KoColorSpaceRegistry::instance()->rgb16("scRGB (linear)");
    Q_ASSERT(linearRgb);

    quint16 src[4];
    src[0] = 257;
    src[1] = 257;
    src[2] = 257;
    src[3] = 65535;

    quint16 dst[4];
    memset(&dst, 0, 8);

    linearRgb->convertPixelsTo((quint8 *)&src, (quint8 *)&dst, sRgb, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::BlackpointCompensation);

    quint16 dst2[4];
    memset(&dst2, 0, 8);

    cmsHPROFILE sRgbProfile = cmsCreate_sRGBProfile();
    QByteArray rawData = linearRgb->profile()->rawData();
    cmsHPROFILE linearRgbProfile = cmsOpenProfileFromMem((void *)rawData.constData(), rawData.size());

    cmsHTRANSFORM tf = cmsCreateTransform(linearRgbProfile,
                                          TYPE_BGRA_16,
                                          sRgbProfile,
                                          TYPE_BGRA_16,
                                          INTENT_RELATIVE_COLORIMETRIC,
                                          cmsFLAGS_NOOPTIMIZE);

    cmsDoTransform(tf, (quint8 *)&src, (quint8 *)&dst2, 1);

    Q_ASSERT(dst[0] == dst2[0]);

}

QTEST_MAIN(TestKoLcmsColorProfile)
