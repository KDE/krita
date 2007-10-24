
#include "kis_rgb_float_hdr_colorspace_test.h"

#include <cmath>

#include <qtest_kde.h>

#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoLcmsRGBColorProfile.h"
#include "../kis_rgb_f32_hdr_colorspace.h"

#ifdef HAVE_OPENEXR
#include "../kis_rgb_f16_hdr_colorspace.h"
#endif

double testRound(double value, const int numDecimalPlaces = 3)
{
    double factor;
    int temp;

    factor = pow(10.0, numDecimalPlaces);
    temp = (int)(value * factor + 0.5);
    return temp / factor;
}

float adjustForExposure(float value, float exposure)
{
    value *= pow(2, exposure + 2.47393);

    // After adjusting by the exposure, map 1.0 to 3.5 f-stops below 1.0
    // I.e. scale by 1/(2^3.5).
    const float middleGreyScaleFactor = 0.0883883;
    value *= middleGreyScaleFactor;

    return value;
}

struct F32Pixel {
    float blue;
    float green;
    float red;
    float alpha;
};

struct U16Pixel {
    quint16 blue;
    quint16 green;
    quint16 red;
    quint16 alpha;
};

quint16 floatToU16(float value)
{
    const int minU16 = 0;
    const int maxU16 = 65535;

    return (quint16)qBound(minU16, qRound(value * maxU16), maxU16);
}

quint8 floatToU8(float value)
{
    const int minU8 = 0;
    const int maxU8 = 255;

    return (quint8)qBound(minU8, qRound(value * maxU8), maxU8);
}

quint16 f32ChannelToU16Channel(float f32Channel, float exposure)
{
    float exposedChannel = adjustForExposure(f32Channel, exposure);

    return floatToU16(exposedChannel);
}

U16Pixel F32PixelToU16Pixel(const F32Pixel f32Pixel, float exposure)
{
    U16Pixel u16Pixel;

    u16Pixel.red = f32ChannelToU16Channel(f32Pixel.red, exposure);
    u16Pixel.green = f32ChannelToU16Channel(f32Pixel.green, exposure);
    u16Pixel.blue = f32ChannelToU16Channel(f32Pixel.blue, exposure);
    u16Pixel.alpha = floatToU16(f32Pixel.alpha);

    return u16Pixel;
}

void KisRgbFloatHDRColorSpaceTest::testProfile()
{
#if 0
    const QString colorSpaceId = KisRgbF32HDRColorSpace::colorSpaceId();

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

    const double gamma = 1.0;

    KoLcmsRGBColorProfile *profile = new KoLcmsRGBColorProfile(chromaticities, gamma);

    KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceId, profile);
    QVERIFY2(colorSpace != 0, "Created colorspace");

    QCOMPARE(colorSpace->profile(), profile);
    QVERIFY(colorSpace->profileIsCompatible(profile));

    const int numPixelsToConvert = 2;

    F32Pixel f32Pixels[numPixelsToConvert];

    f32Pixels[0].red = 5.0;
    f32Pixels[0].green = 0.5;
    f32Pixels[0].blue = 50;
    f32Pixels[0].alpha = 0.75;

    f32Pixels[1].red = 1.0;
    f32Pixels[1].green = -1;
    f32Pixels[1].blue = 0.18;
    f32Pixels[1].alpha = 1.0;

    const float exposure = 1.0;

    U16Pixel u16Pixels[numPixelsToConvert];

    for (int pixel = 0; pixel < numPixelsToConvert; pixel++) {
        u16Pixels[pixel] = F32PixelToU16Pixel(f32Pixels[pixel], exposure);
    }

    KoLcmsColorProfile *destinationProfile = new KoLcmsColorProfile(cmsCreate_sRGBProfile());
    cmsHPROFILE destinationLcmsProfile = destinationProfile->lcmsProfile();

    const DWORD inputFormat = TYPE_BGRA_16;
    const DWORD outputFormat = TYPE_BGRA_8;
    const int renderingIntent = INTENT_PERCEPTUAL;
    const DWORD flags = cmsFLAGS_NOTPRECALC;

    cmsHTRANSFORM transform = cmsCreateTransform(profile->lcmsProfile(), 
                                                 inputFormat, 
                                                 destinationLcmsProfile, 
                                                 outputFormat, 
                                                 renderingIntent, 
                                                 flags);
    struct U8Pixel {
        quint8 blue;
        quint8 green;
        quint8 red;
        quint8 alpha;
    };

    U8Pixel u8Pixels[numPixelsToConvert];

    cmsDoTransform(transform, &u16Pixels, &u8Pixels, numPixelsToConvert);

    for (int pixel = 0; pixel < numPixelsToConvert; pixel++) {
        u8Pixels[pixel].alpha = floatToU8(f32Pixels[pixel].alpha);
    }

    const int imageWidth = numPixelsToConvert;
    const int imageHeight = 1;

    QImage image = colorSpace->convertToQImage((const quint8 *)&f32Pixels, 
                                               imageWidth, 
                                               imageHeight, 
                                               destinationProfile, 
                                               (KoColorConversionTransformation::Intent)renderingIntent, 
                                               exposure);

    QCOMPARE(image.width(), imageWidth);
    QCOMPARE(image.height(), imageHeight);

    int pixelX = 0;
    const int pixelY = 0;

    QRgb imagePixel = image.pixel(pixelX, pixelY);

    QCOMPARE(qRed(imagePixel), (int)u8Pixels[pixelX].red);
    QCOMPARE(qGreen(imagePixel), (int)u8Pixels[pixelX].green);
    QCOMPARE(qBlue(imagePixel), (int)u8Pixels[pixelX].blue);
    QCOMPARE(qAlpha(imagePixel), (int)u8Pixels[pixelX].alpha);

    pixelX = 1;
    imagePixel = image.pixel(pixelX, pixelY);

    QCOMPARE(qRed(imagePixel), (int)u8Pixels[pixelX].red);
    QCOMPARE(qGreen(imagePixel), (int)u8Pixels[pixelX].green);
    QCOMPARE(qBlue(imagePixel), (int)u8Pixels[pixelX].blue);
    QCOMPARE(qAlpha(imagePixel), (int)u8Pixels[pixelX].alpha);
#endif
}

void KisRgbFloatHDRColorSpaceTest::testFactory()
{
#if 0
    QString colorSpaceId;
#ifdef HAVE_OPENEXR
    colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID, Float16BitsColorDepthID);
    QCOMPARE(colorSpaceId, KisRgbF16HDRColorSpace::colorSpaceId());
#endif

    colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID, Float32BitsColorDepthID);
    QCOMPARE(colorSpaceId, KisRgbF32HDRColorSpace::colorSpaceId());

    KoLcmsRGBColorProfile::Chromaticities chromaticities;

    chromaticities.primaries.Red.x = 0.6400f;
    chromaticities.primaries.Red.y = 0.3300f;
    chromaticities.primaries.Red.Y = 1.0f;
    chromaticities.primaries.Green.x = 0.3000f;
    chromaticities.primaries.Green.y = 0.6000f;
    chromaticities.primaries.Green.Y = 1.0f;
    chromaticities.primaries.Blue.x = 0.1500f;
    chromaticities.primaries.Blue.y = 0.0600f;
    chromaticities.primaries.Blue.Y = 1.0f;
    chromaticities.whitePoint.x = 0.3127f;
    chromaticities.whitePoint.y = 0.3290f;
    chromaticities.whitePoint.Y = 1.0f;

    const double gamma = 1.0;

    KoLcmsRGBColorProfile *profile = new KoLcmsRGBColorProfile(chromaticities, gamma);

    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(colorSpaceId);
    QVERIFY(colorSpaceFactory);

    QVERIFY(colorSpaceFactory->profileIsCompatible(profile));

    KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceId, 0);
    QVERIFY2(colorSpace != 0, "Created colorspace");
    QVERIFY2(colorSpace->profile() != 0, "Has a profile by default");

    KoLcmsColorProfile *lcmsProfile = static_cast<KoLcmsColorProfile *>(colorSpace->profile());

    LPGAMMATABLE redGamma = cmsReadICCGamma(lcmsProfile->lcmsProfile(), icSigRedTRCTag);

    QCOMPARE(testRound(cmsEstimateGamma(redGamma)), gamma);
#endif
}

template <class ColorSpaceTraits>
void KisRgbFloatHDRColorSpaceTest::testChannels(const QString &colorSpaceId, 
                                                const KoChannelInfo::enumChannelValueType channelValueType)
{
    KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceId, 0);
    QVERIFY(colorSpace != 0);

    QList<KoChannelInfo*> channels = colorSpace->channels();

    const int numChannels = 4;

    QCOMPARE(channels.size(), numChannels);

    const int redChannelIndex = 0;
    const int greenChannelIndex = 1;
    const int blueChannelIndex = 2;
    const int alphaChannelIndex = 3;

    QCOMPARE(channels[redChannelIndex]->name(), i18n("Red"));
    QCOMPARE(channels[redChannelIndex]->pos(), (int)(ColorSpaceTraits::red_pos * sizeof(typename ColorSpaceTraits::channels_type)));
    QCOMPARE(channels[redChannelIndex]->color(), QColor(255, 0, 0));

    QCOMPARE(channels[greenChannelIndex]->name(), i18n("Green"));
    QCOMPARE(channels[greenChannelIndex]->pos(), (int)(ColorSpaceTraits::green_pos * sizeof(typename ColorSpaceTraits::channels_type)));
    QCOMPARE(channels[greenChannelIndex]->color(), QColor(0, 255, 0));

    QCOMPARE(channels[blueChannelIndex]->name(), i18n("Blue"));
    QCOMPARE(channels[blueChannelIndex]->pos(), (int)(ColorSpaceTraits::blue_pos * sizeof(typename ColorSpaceTraits::channels_type)));
    QCOMPARE(channels[blueChannelIndex]->color(), QColor(0, 0, 255));

    QCOMPARE(channels[alphaChannelIndex]->name(), i18n("Alpha"));
    QCOMPARE(channels[alphaChannelIndex]->pos(), (int)(ColorSpaceTraits::alpha_pos * sizeof(typename ColorSpaceTraits::channels_type)));
    QCOMPARE(channels[alphaChannelIndex]->color(), QColor(0, 0, 0));

    const int numColorChannels = 3;

    for (int channelIndex = 0; channelIndex < numColorChannels; channelIndex++)
    {
        QCOMPARE(channels[channelIndex]->channelType(), KoChannelInfo::COLOR);
    }

    QCOMPARE(channels[alphaChannelIndex]->channelType(), KoChannelInfo::ALPHA);

    for (int channelIndex = 0; channelIndex < numChannels; channelIndex++)
    {
        QCOMPARE(channels[channelIndex]->channelValueType(), channelValueType);
        QCOMPARE(channels[channelIndex]->size(), (int)sizeof(typename ColorSpaceTraits::channels_type));
    }
}

void KisRgbFloatHDRColorSpaceTest::testF32Channels()
{
    testChannels<RgbF32Traits>(KisRgbF32HDRColorSpace::colorSpaceId(), KoChannelInfo::FLOAT32);
}

#ifdef HAVE_OPENEXR
void KisRgbFloatHDRColorSpaceTest::testF16Channels()
{
    testChannels<RgbF16Traits>(KisRgbF16HDRColorSpace::colorSpaceId(), KoChannelInfo::FLOAT16);
}
#endif

QTEST_KDEMAIN(KisRgbFloatHDRColorSpaceTest, NoGUI)
#include "kis_rgb_float_hdr_colorspace_test.moc"

