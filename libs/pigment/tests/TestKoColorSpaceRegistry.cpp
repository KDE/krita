
#include "TestKoColorSpaceRegistry.h"

#include <qtest_kde.h>
#include <kdebug.h>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"
#include "KoRgbU8ColorSpace.h"
#include "KoRgbU16ColorSpace.h"
#include "KoLabColorSpace.h"

void TestKoColorSpaceRegistry::testRgbU8()
{
    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(KoRgbU8ColorSpace::colorSpaceId());
    QVERIFY(colorSpaceFactory != 0);

    KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    QVERIFY(colorSpace != 0);
    QCOMPARE(colorSpace->id(), KoRgbU8ColorSpace::colorSpaceId());

    KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU8ProfileName";

    cmsAddTag(lcmsProfile, icSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsAddTag(lcmsProfile, icSigDeviceModelDescTag, testProfileName.toLatin1().data());
    QByteArray manufacturerName("");
    cmsAddTag(lcmsProfile, icSigDeviceMfgDescTag, manufacturerName.data());

    KoColorProfile *testProfile = new KoLcmsColorProfile(lcmsProfile);
    KoColorSpaceRegistry::instance()->addProfile(testProfile);

    colorSpace = KoColorSpaceRegistry::instance()->rgb8(testProfileName);
    QVERIFY(colorSpace != 0);

    profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), testProfileName);

    colorSpace = KoColorSpaceRegistry::instance()->rgb8(testProfile);
    QVERIFY(colorSpace != 0);

    profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile, testProfile);
}

void TestKoColorSpaceRegistry::testRgbU16()
{
    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(KoRgbU16ColorSpace::colorSpaceId());
    QVERIFY(colorSpaceFactory != 0);

    KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb16();
    QVERIFY(colorSpace != 0);
    QCOMPARE(colorSpace->id(), KoRgbU16ColorSpace::colorSpaceId());

    KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU16ProfileName";

    cmsAddTag(lcmsProfile, icSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsAddTag(lcmsProfile, icSigDeviceModelDescTag, testProfileName.toLatin1().data());
    QByteArray manufacturerName("");
    cmsAddTag(lcmsProfile, icSigDeviceMfgDescTag, manufacturerName.data());

    KoColorProfile *testProfile = new KoLcmsColorProfile(lcmsProfile);
    KoColorSpaceRegistry::instance()->addProfile(testProfile);

    colorSpace = KoColorSpaceRegistry::instance()->rgb16(testProfileName);
    QVERIFY(colorSpace != 0);

    profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), testProfileName);

    colorSpace = KoColorSpaceRegistry::instance()->rgb16(testProfile);
    QVERIFY(colorSpace != 0);

    profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile, testProfile);
}

void TestKoColorSpaceRegistry::testLab()
{
    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(KoLabColorSpace::colorSpaceId());
    QVERIFY(colorSpaceFactory != 0);

    KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->lab16();
    QVERIFY(colorSpace != 0);
    QCOMPARE(colorSpace->id(), KoLabColorSpace::colorSpaceId());

    KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsCIExyY whitepoint;
    whitepoint.x = 0.33;
    whitepoint.y = 0.33;
    whitepoint.Y = 1.0;

    cmsHPROFILE lcmsProfile = cmsCreateLabProfile(&whitepoint);
    QString testProfileName = "TestLabProfileName";

    cmsAddTag(lcmsProfile, icSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsAddTag(lcmsProfile, icSigDeviceModelDescTag, testProfileName.toLatin1().data());
    QByteArray manufacturerName("");
    cmsAddTag(lcmsProfile, icSigDeviceMfgDescTag, manufacturerName.data());

    KoColorProfile *testProfile = new KoLcmsColorProfile(lcmsProfile);
    KoColorSpaceRegistry::instance()->addProfile(testProfile);

    colorSpace = KoColorSpaceRegistry::instance()->lab16(testProfileName);
    QVERIFY(colorSpace != 0);

    profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), testProfileName);

    colorSpace = KoColorSpaceRegistry::instance()->lab16(testProfile);
    QVERIFY(colorSpace != 0);

    profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile, testProfile);
}

QTEST_KDEMAIN(TestKoColorSpaceRegistry, NoGUI)
#include "TestKoColorSpaceRegistry.moc"

