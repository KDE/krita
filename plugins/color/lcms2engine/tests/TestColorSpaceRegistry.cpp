
#include "TestColorSpaceRegistry.h"

#include <QTest>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"
#include "RgbU8ColorSpace.h"
#include "RgbU16ColorSpace.h"
#include "LabColorSpace.h"

#include "sdk/tests/kistest.h"

void TestColorSpaceRegistry::testConstruction()
{
    KoColorSpaceRegistry *instance = KoColorSpaceRegistry::instance();
    Q_ASSERT(instance);
}

void TestColorSpaceRegistry::testRgbU8()
{
    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID,
                                                                                Integer8BitsColorDepthID);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId));

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU8ProfileName";

    cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag, testProfileName.toLatin1().constData());
    cmsWriteTag(lcmsProfile, cmsSigDeviceModelDescTag, testProfileName.toLatin1().constData());
    cmsWriteTag(lcmsProfile, cmsSigDeviceMfgDescTag, "");

}

void TestColorSpaceRegistry::testRgbU16()
{
    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID,
                                                                                Integer16BitsColorDepthID);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb16();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId));

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU16ProfileName";

    cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag, testProfileName.toLatin1().constData());
    cmsWriteTag(lcmsProfile, cmsSigDeviceModelDescTag, testProfileName.toLatin1().constData());
    cmsWriteTag(lcmsProfile, cmsSigDeviceMfgDescTag, "");

}

void TestColorSpaceRegistry::testLab()
{
    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(LABAColorModelID,
                                                                                Integer16BitsColorDepthID);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->lab16();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId));

    cmsCIExyY whitepoint;
    whitepoint.x = 0.33;
    whitepoint.y = 0.33;
    whitepoint.Y = 1.0;

    cmsHPROFILE lcmsProfile = cmsCreateLab2Profile(&whitepoint);
    QString testProfileName = "TestLabProfileName";

    cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag, testProfileName.toLatin1().constData());
    cmsWriteTag(lcmsProfile, cmsSigDeviceModelDescTag, testProfileName.toLatin1().constData());
    cmsWriteTag(lcmsProfile, cmsSigDeviceMfgDescTag, "");

}

KISTEST_MAIN(TestColorSpaceRegistry)
