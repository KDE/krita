
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

void TestKoLcmsColorProfile::testProofingConversion()
{
    const KoColorSpace *sRgb = KoColorSpaceRegistry::instance()->rgb16("sRGB built-in");
    Q_ASSERT(sRgb);
    const KoColorSpace *lab = KoColorSpaceRegistry::instance()->lab16();//there's only one lab profile, replace with it's name.
    Q_ASSERT(lab);

    quint16 src[4];//the following ought to give us a purple only possible in lab. I can't seem to proof this away, somehow...
    src[0] = 32896;
    src[1] = 65535;
    src[2] = 0;
    src[3] = 65535;

    quint16 dst[4];
    memset(&dst, 0, 8);

    cmsHPROFILE sRgbProfile = cmsCreate_sRGBProfile();
    cmsHPROFILE LabProfile = cmsCreateLab4Profile(NULL);

    quint16 alarm[cmsMAXCHANNELS]={0};
    alarm[0] = 65535;
    alarm[1] = 0;
    alarm[2] = 0;
    alarm[3] = 65535;
    cmsSetAlarmCodes(alarm);

    cmsHTRANSFORM tf = cmsCreateProofingTransform(LabProfile,
                                          TYPE_Lab_16,
                                          LabProfile,
                                          TYPE_Lab_16,
                                          sRgbProfile,
                                          INTENT_ABSOLUTE_COLORIMETRIC,
                                          INTENT_ABSOLUTE_COLORIMETRIC,
                                          cmsFLAGS_SOFTPROOFING|cmsFLAGS_GAMUTCHECK);

    cmsDoTransform(tf, (quint8 *)&src, (quint8 *)&dst, 1);

    qDebug()<<dst[0]<<","<<dst[1]<<","<<dst[2]<<","<<dst[3];
    Q_ASSERT((dst[0] == alarm[0]) && (dst[1] == alarm[1]) && (dst[2] == alarm[2]));

}
QTEST_MAIN(TestKoLcmsColorProfile)
