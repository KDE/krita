/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_lms_f32_colorspace.h"

#include <klocale.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <KoCtlColorProfile.h>

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

KoID LMSAColorModelID("LMSA", i18n("Long Middle Short Cone Space"));

KisLmsAF32ColorSpace::KisLmsAF32ColorSpace(const KoCtlColorProfile *p) : KoCtlMonoTypeColorSpace<KisLmsAF32Traits>("LMSAF32", i18n("LMS Cone Space (32-bit float/channel)"), KoColorSpaceRegistry::instance()->rgb16(), p)
{
    const int channelSize = sizeof(float);
    const KoChannelInfo::enumChannelValueType channelValueType = KoColorSpaceMathsTraits<KisLmsAF32Traits::channels_type>::channelValueType;
    this->addChannel(new KoChannelInfo(i18n("L"),
                                       0 * channelSize,
                                       KoChannelInfo::COLOR,
                                       channelValueType,
                                       channelSize,
                                       QColor(255, 0, 0)));
    this->addChannel(new KoChannelInfo(i18n("M"),
                                       1 * channelSize,
                                       KoChannelInfo::COLOR,
                                       channelValueType,
                                       channelSize,
                                       QColor(0, 255, 0)));
    this->addChannel(new KoChannelInfo(i18n("S"),
                                       2 * channelSize,
                                       KoChannelInfo::COLOR,
                                       channelValueType,
                                       channelSize,
                                       QColor(0, 0, 255)));
    this->addChannel(new KoChannelInfo(i18n("Alpha"),
                                       3 * channelSize,
                                       KoChannelInfo::ALPHA,
                                       channelValueType,
                                       channelSize));

    addCompositeOp(new KoCompositeOpOver<KisLmsAF32Traits>(this));
    addCompositeOp(new KoCompositeOpErase<KisLmsAF32Traits>(this));
}

KisLmsAF32ColorSpace::~KisLmsAF32ColorSpace()
{
}

KoColorSpace* KisLmsAF32ColorSpace::clone() const
{
    return new KisLmsAF32ColorSpace(static_cast<const KoCtlColorProfile *>(profile()));
}

KoID KisLmsAF32ColorSpace::colorModelId() const
{
    return KoID("LMSA", i18n("Long Middle Short Cone Space"));
}

KoID KisLmsAF32ColorSpace::colorDepthId() const
{
    return Float32BitsColorDepthID;
}

bool KisLmsAF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    Q_UNUSED(independence);
    return true;
}

bool KisLmsAF32ColorSpace::hasHighDynamicRange() const
{
    return false;
}

KoColorSpace *KisLmsAF32ColorSpaceFactory::createColorSpace(const KoColorProfile *p) const
{
    return new KisLmsAF32ColorSpace(dynamic_cast<const KoCtlColorProfile*>(p));
}

QString KisLmsAF32ColorSpaceFactory::id() const
{
    return "LMSAF32";
}

QString KisLmsAF32ColorSpaceFactory::name() const
{
    return i18n("Long Middle Short Cone Space (32-bit float/channel)");
}

bool KisLmsAF32ColorSpaceFactory::userVisible() const
{
    return true;
}

KoID KisLmsAF32ColorSpaceFactory::colorModelId() const
{
    return LMSAColorModelID;
}

KoID KisLmsAF32ColorSpaceFactory::colorDepthId() const
{
    return Float32BitsColorDepthID;
}

QString KisLmsAF32ColorSpaceFactory::defaultProfile()
{
    return QString("Standard LMS");
}

bool KisLmsAF32ColorSpaceFactory::profileIsCompatible(const KoColorProfile* profile) const
{
    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(profile);
    if (ctlp && ctlp->colorModel() == "LMSA" && ctlp->colorDepth() == "F32") {
        return true;
    }
    return false;
}

QString KisLmsAF32ColorSpaceFactory::colorSpaceEngine() const
{
    return "";
}

bool KisLmsAF32ColorSpaceFactory::isHdr() const
{
    return true;
}

int KisLmsAF32ColorSpaceFactory::referenceDepth() const
{
    return 32;
}

KoColorConversionTransformationFactory* KisLmsAF32ColorSpaceFactory::createICCColorConversionTransformationFactory(const QString& profileName) const
{
    Q_UNUSED(profileName);
    return 0;
}

QString KisLmsAF32ColorSpaceFactory::defaultProfile() const
{
    return "";
}


#if 0

#include <limits.h>

#include <QImage>

#include <kis_debug.h>
#include <klocale.h>


#include "kis_color_conversions.h"

namespace
{
const qint32 MAX_CHANNEL_LMS = 3;
const qint32 MAX_CHANNEL_LMSA = 4;
}

#include "KoIntegerMaths.h"

#define FLOAT_MAX 1.0f //temp

#define EPSILON 1e-6

// XXX: lcms doesn't support 32-bit float
#define F32_LCMS_TYPE TYPE_BGRA_16

// disable the lcms handling by setting profile=0
KisLmsF32ColorSpace::KisLmsF32ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile */*p*/) :
        KoColorSpace("LMSAF32", i18n("LMS (32-bit float/channel)"), parent)
        , KoF32ColorSpaceTrait(PIXEL_ALPHA * sizeof(float))
        , KoLcmsColorSpaceTrait(F32_LCMS_TYPE, icSig3colorData, 0)
{
    addChannel(new KoChannelInfo(i18n("Long"), PIXEL_LONGWAVE * sizeof(float), KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float)));
    addChannel(new KoChannelInfo(i18n("Middle"), PIXEL_MIDDLEWAVE * sizeof(float), KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float)));
    addChannel(new KoChannelInfo(i18n("Short"), PIXEL_SHORTWAVE * sizeof(float), KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float)));
    addChannel(new KoChannelInfo(i18n("Alpha"), PIXEL_ALPHA * sizeof(float), KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));
}

KisLmsF32ColorSpace::~KisLmsF32ColorSpace()
{
}

void KisLmsF32ColorSpace::setPixel(quint8 *dst, float longWave, float middleWave, float shortWave, float alpha) const
{
    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->longWave = longWave;
    dstPixel->middleWave = middleWave;
    dstPixel->shortWave = shortWave;
    dstPixel->alpha = alpha;
}

void KisLmsF32ColorSpace::getPixel(const quint8 *src, float *longWave, float *middleWave, float *shortWave, float *alpha) const
{
    const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);

    *longWave = srcPixel->longWave;
    *middleWave = srcPixel->middleWave;
    *shortWave = srcPixel->shortWave;
    *alpha = srcPixel->alpha;
}

void KisLmsF32ColorSpace::fromQColor(const QColor& c, quint8 *dstU8, KoColorProfile * /*profile*/)
{
    Pixel *dst = reinterpret_cast<Pixel *>(dstU8);

    dst->longWave = computeLong(c.red(), c.green(), c.blue());
    dst->middleWave = computeMiddle(c.red(), c.green(), c.blue());
    dst->shortWave = computeShort(c.red(), c.green(), c.blue());
}

void KisLmsF32ColorSpace::fromQColor(const QColor& c, quint8 opacity, quint8 *dstU8, KoColorProfile * /*profile*/)
{
    Pixel *dst = reinterpret_cast<Pixel *>(dstU8);

    dst->longWave = computeLong(c.red(), c.green(), c.blue());
    dst->middleWave = computeMiddle(c.red(), c.green(), c.blue());
    dst->shortWave = computeShort(c.red(), c.green(), c.blue());
    dst->alpha = UINT8_TO_FLOAT(opacity);
}

void KisLmsF32ColorSpace::toQColor(const quint8 *srcU8, QColor *c, KoColorProfile * /*profile*/)
{
    const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);

    c->setRgb(computeRed(src->longWave, src->middleWave, src->shortWave), computeGreen(src->longWave, src->middleWave, src->shortWave), computeBlue(src->longWave, src->middleWave, src->shortWave));
}

void KisLmsF32ColorSpace::toQColor(const quint8 *srcU8, QColor *c, quint8 *opacity, KoColorProfile * /*profile*/)
{
    const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);

    c->setRgb(computeRed(src->longWave, src->middleWave, src->shortWave), computeGreen(src->longWave, src->middleWave, src->shortWave), computeBlue(src->longWave, src->middleWave, src->shortWave));
    *opacity = FLOAT_TO_UINT8(src->alpha);
}

quint8 KisLmsF32ColorSpace::difference(const quint8 *src1U8, const quint8 *src2U8)
{
    const Pixel *src1 = reinterpret_cast<const Pixel *>(src1U8);
    const Pixel *src2 = reinterpret_cast<const Pixel *>(src2U8);

    return FLOAT_TO_UINT8(qMax(qAbs(src2->longWave - src1->longWave),
                               qMax(qAbs(src2->middleWave - src1->middleWave),
                                    qAbs(src2->shortWave - src1->shortWave))));
}

void KisLmsF32ColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    float totalLong = 0, totalMiddle = 0, totalShort = 0, newAlpha = 0;

    while (nColors--) {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        float alpha = pixel->alpha;
        float alphaTimesWeight = alpha * UINT8_TO_FLOAT(*weights);

        totalLong += pixel->longWave * alphaTimesWeight;
        totalMiddle += pixel->middleWave * alphaTimesWeight;
        totalShort += pixel->shortWave * alphaTimesWeight;
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= F32_OPACITY_OPAQUE);

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > EPSILON) {
        totalLong = totalLong / newAlpha;
        totalMiddle = totalMiddle / newAlpha;
        totalShort = totalShort / newAlpha;
    }

    dstPixel->longWave = totalLong;
    dstPixel->middleWave = totalMiddle;
    dstPixel->shortWave = totalShort;
}

QVector<KoChannelInfo *> KisLmsF32ColorSpace::channels() const
{
    return m_channels;
}

quint32 KisLmsF32ColorSpace::channelCount() const
{
    return MAX_CHANNEL_LMSA;
}

quint32 KisLmsF32ColorSpace::colorChannelCount() const
{
    return MAX_CHANNEL_LMS;
}

quint32 KisLmsF32ColorSpace::pixelSize() const
{
    return MAX_CHANNEL_LMSA * sizeof(float);
}

QImage KisLmsF32ColorSpace::convertToQImage(const quint8 *dataU8, qint32 width, qint32 height,
        KoColorProfile *  /*dstProfile*/,
        qint32 /*renderingIntent*/, float /*exposure*/)

{
    const float *data = reinterpret_cast<const float *>(dataU8);

    QImage image = QImage(width, height, QImage::Format_ARGB32);

    qint32 i = 0;
    uchar *j = image.bits();

    while (i < width * height * MAX_CHANNEL_LMSA) {
        double l = *(data + i + PIXEL_LONGWAVE);
        double m = *(data + i + PIXEL_MIDDLEWAVE);
        double s = *(data + i + PIXEL_SHORTWAVE);
        *(j + 3)  = FLOAT_TO_UINT8(*(data + i + PIXEL_ALPHA));
        *(j + 2) = computeRed(l, m, s);
        *(j + 1) = computeGreen(l, m, s);
        *(j + 0) = computeBlue(l, m, s);
        i += MAX_CHANNEL_LMSA;
        j += MAX_CHANNEL_LMSA;
    }

    /*
    if (srcProfile != 0 && dstProfile != 0) {
        convertPixelsTo(image.bits(), srcProfile,
                image.bits(), this, dstProfile,
                width * height, renderingIntent);
    }
    */
    return image;
}


void KisLmsF32ColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    while (rows > 0) {

        const float *src = reinterpret_cast<const float *>(srcRowStart);
        float *dst = reinterpret_cast<float *>(dstRowStart);
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            float srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                quint8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha *= UINT8_TO_FLOAT(U8_mask);
                }
                mask++;
            }

            if (srcAlpha > F32_OPACITY_TRANSPARENT + EPSILON) {

                if (opacity < F32_OPACITY_OPAQUE - EPSILON) {
                    srcAlpha *= opacity;
                }

                if (srcAlpha > F32_OPACITY_OPAQUE - EPSILON) {
                    memcpy(dst, src, MAX_CHANNEL_LMSA * sizeof(float));
                } else {
                    float dstAlpha = dst[PIXEL_ALPHA];

                    float srcBlend;

                    if (dstAlpha > F32_OPACITY_OPAQUE - EPSILON) {
                        srcBlend = srcAlpha;
                    } else {
                        float newAlpha = dstAlpha + (F32_OPACITY_OPAQUE - dstAlpha) * srcAlpha;
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha > EPSILON) {
                            srcBlend = srcAlpha / newAlpha;
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend > F32_OPACITY_OPAQUE - EPSILON) {
                        memcpy(dst, src, MAX_CHANNEL_LMS * sizeof(float));
                    } else {
                        dst[PIXEL_LONGWAVE] = FLOAT_BLEND(src[PIXEL_LONGWAVE], dst[PIXEL_LONGWAVE], srcBlend);
                        dst[PIXEL_MIDDLEWAVE] = FLOAT_BLEND(src[PIXEL_MIDDLEWAVE], dst[PIXEL_MIDDLEWAVE], srcBlend);
                        dst[PIXEL_SHORTWAVE] = FLOAT_BLEND(src[PIXEL_SHORTWAVE], dst[PIXEL_SHORTWAVE], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_LMSA;
            dst += MAX_CHANNEL_LMSA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart) {
            maskRowStart += maskRowStride;
        }
    }
}

void KisLmsF32ColorSpace::compositeErase(quint8 *dst,
        qint32 dstRowSize,
        const quint8 *src,
        qint32 srcRowSize,
        const quint8 *srcAlphaMask,
        qint32 maskRowStride,
        qint32 rows,
        qint32 cols,
        float /*opacity*/)
{
    while (rows-- > 0) {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const quint8 *mask = srcAlphaMask;

        for (qint32 i = cols; i > 0; i--, s++, d++) {
            float srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                quint8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = FLOAT_BLEND(srcAlpha, F32_OPACITY_OPAQUE, UINT8_TO_FLOAT(U8_mask));
                }
                mask++;
            }
            d->alpha = srcAlpha * d->alpha;
        }

        dst += dstRowSize;
        src += srcRowSize;
        if (srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisLmsF32ColorSpace::compositeCopy(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride,
                                        const quint8 */*maskRowStart*/, qint32 /*maskRowStride*/, qint32 rows, qint32 numColumns, float /*opacity*/)
{
    while (rows > 0) {
        memcpy(dstRowStart, srcRowStart, numColumns * sizeof(Pixel));
        --rows;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
    }
}

void KisLmsF32ColorSpace::bitBlt(quint8 *dst,
                                 qint32 dstRowStride,
                                 const quint8 *src,
                                 qint32 srcRowStride,
                                 const quint8 *mask,
                                 qint32 maskRowStride,
                                 quint8 U8_opacity,
                                 qint32 rows,
                                 qint32 cols,
                                 const KoCompositeOp* op)
{
    float opacity = UINT8_TO_FLOAT(U8_opacity);

    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    default:
        break;
    }
}

KoCompositeOpList KisLmsF32ColorSpace::compositeOps() const
{
    KoCompositeOpList list;

    list.append(KoCompositeOp(COMPOSITE_OVER));
    return list;
}

#endif
