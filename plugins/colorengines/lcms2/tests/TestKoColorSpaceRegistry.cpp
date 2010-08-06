
#include "TestKoColorSpaceRegistry.h"

#include <qtest_kde.h>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"
#include "RgbU8ColorSpace.h"
#include "RgbU16ColorSpace.h"
#include "LabColorSpace.h"

void TestKoColorSpaceRegistry::testConstruction()
{
    KoColorSpaceRegistry* instance = KoColorSpaceRegistry::instance();
    Q_ASSERT(instance);
}

void TestKoColorSpaceRegistry::testRgbU8()
{
    QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID,
                           Integer8BitsColorDepthID);
    const KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->colorSpaceFactory(colorSpaceId);
    QVERIFY(colorSpaceFactory != 0);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU8ProfileName";

    cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsWriteTag(lcmsProfile, cmsSigDeviceModelDescTag, testProfileName.toLatin1().data());
    cmsWriteTag(lcmsProfile, cmsSigDeviceMfgDescTag, "");

}

void TestKoColorSpaceRegistry::testRgbU16()
{
    QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID,
                           Integer16BitsColorDepthID);
    const KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->colorSpaceFactory(colorSpaceId);
    QVERIFY(colorSpaceFactory != 0);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb16();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU16ProfileName";

    cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsWriteTag(lcmsProfile, cmsSigDeviceModelDescTag, testProfileName.toLatin1().data());
    cmsWriteTag(lcmsProfile, cmsSigDeviceMfgDescTag, "");

}

void TestKoColorSpaceRegistry::testLab()
{
    QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(LABAColorModelID,
                           Integer16BitsColorDepthID);
    const KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->colorSpaceFactory(colorSpaceId);
    QVERIFY(colorSpaceFactory != 0);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->lab16();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsCIExyY whitepoint;
    whitepoint.x = 0.33;
    whitepoint.y = 0.33;
    whitepoint.Y = 1.0;

    cmsHPROFILE lcmsProfile = cmsCreateLab2Profile(&whitepoint);
    QString testProfileName = "TestLabProfileName";

    cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsWriteTag(lcmsProfile, cmsSigDeviceModelDescTag, testProfileName.toLatin1().data());
    cmsWriteTag(lcmsProfile, cmsSigDeviceMfgDescTag, "");

}

QTEST_KDEMAIN(TestKoColorSpaceRegistry, NoGUI)
#include <TestKoColorSpaceRegistry.moc>

