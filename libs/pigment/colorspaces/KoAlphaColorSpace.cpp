/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "KoAlphaColorSpace.h"

#include <limits.h>
#include <stdlib.h>

#include <QImage>
#include <QBitArray>

#include <klocalizedstring.h>

#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"
#include "KoCompositeOpOver.h"
#include "KoCompositeOpErase.h"
#include "KoCompositeOpCopy2.h"
#include "KoCompositeOpAlphaDarken.h"
#include "KoCompositeOpBase.h"
#include "KoCompositeOps.h"
#include <colorprofiles/KoDummyColorProfile.h>

namespace {
template <typename channel_type> KoChannelInfo::enumChannelValueType channelInfoIdFromChannelType();
template <> inline KoChannelInfo::enumChannelValueType channelInfoIdFromChannelType<quint8>() { return KoChannelInfo::UINT8; }
template <> inline KoChannelInfo::enumChannelValueType channelInfoIdFromChannelType<quint16>() { return KoChannelInfo::UINT16; }
#ifdef HAVE_OPENEXR
template <> inline KoChannelInfo::enumChannelValueType channelInfoIdFromChannelType<half>() { return KoChannelInfo::FLOAT16; }
#endif
template <> inline KoChannelInfo::enumChannelValueType channelInfoIdFromChannelType<float>() { return KoChannelInfo::FLOAT32; }
}

template<class Traits>
class AlphaColorSpaceMultiplyOp : public KoCompositeOpBase< Traits, AlphaColorSpaceMultiplyOp<Traits>>
{
    typedef KoCompositeOpBase<Traits, AlphaColorSpaceMultiplyOp<Traits>> base_class;
    typedef typename Traits::channels_type channels_type;

public:
    AlphaColorSpaceMultiplyOp(const KoColorSpace* cs)
        : base_class(cs, COMPOSITE_MULT, KoCompositeOp::categoryArithmetic()) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        using namespace Arithmetic;

        Q_UNUSED(allChannelFlags);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(channelFlags);

        if (!alphaLocked) {
            // use internal parallelism for multiplication!
            srcAlpha = mul(srcAlpha, maskAlpha);
            dstAlpha = mul(dstAlpha, opacity);
            dstAlpha = mul(srcAlpha, dstAlpha);
        }

        return dstAlpha;
    }
};


template <class _CSTrait>
KoAlphaColorSpaceImpl<_CSTrait>::KoAlphaColorSpaceImpl()
    : KoColorSpaceAbstract<_CSTrait>(alphaIdFromChannelType<channels_type>().id(),
                                     alphaIdFromChannelType<channels_type>().name())
{
    this->addChannel(new KoChannelInfo(i18n("Alpha"), 0, 0, KoChannelInfo::ALPHA, channelInfoIdFromChannelType<channels_type>()));

    m_compositeOps << new KoCompositeOpOver<_CSTrait>(this)
                   << new KoCompositeOpErase<_CSTrait>(this)
                   << new KoCompositeOpCopy2<_CSTrait>(this)
                   << createAlphaDarkenCompositeOp<_CSTrait>(this)
                   << new AlphaColorSpaceMultiplyOp<_CSTrait>(this);

    Q_FOREACH (KoCompositeOp *op, m_compositeOps) {
        this->addCompositeOp(op);
    }
    m_profile = new KoDummyColorProfile;
}

template <class _CSTrait>
KoAlphaColorSpaceImpl<_CSTrait>::~KoAlphaColorSpaceImpl()
{
    qDeleteAll(m_compositeOps);
    delete m_profile;
    m_profile = 0;
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
    _CSTrait::nativeArray(dst)[0] = _MathsFromU8::scaleToA(c.alpha());
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    c->setRgba(qRgba(255, 255, 255, _MathsToU8::scaleToA(_CSTrait::nativeArray(src)[0])));
}

template <class _CSTrait>
quint8 KoAlphaColorSpaceImpl<_CSTrait>::difference(const quint8 *src1, const quint8 *src2) const
{
    return qAbs(_MathsToU8::scaleToA(_CSTrait::nativeArray(src2)[0] - _CSTrait::nativeArray(src1)[0]));
}

template <class _CSTrait>
quint8 KoAlphaColorSpaceImpl<_CSTrait>::differenceA(const quint8 *src1, const quint8 *src2) const
{
    return difference(src1, src2);
}

template <class _CSTrait>
QString KoAlphaColorSpaceImpl<_CSTrait>::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < this->channelCount());
    const quint32 channelPosition = this->channels()[channelIndex]->pos();
    return QString().setNum(_CSTrait::nativeArray(pixel)[channelPosition]);
}

template <class _CSTrait>
QString KoAlphaColorSpaceImpl<_CSTrait>::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < this->channelCount());
    const quint32 channelPosition = this->channels()[channelIndex]->pos();
    return QString().setNum(KoColorSpaceMaths<channels_type, float>::scaleToA(_CSTrait::nativeArray(pixel)[channelPosition]));
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::convolveColors(quint8** colors, qreal * kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nColors, const QBitArray & channelFlags) const
{
    qreal totalAlpha = 0;

    while (nColors--) {
        qreal weight = *kernelValues;

        if (weight != 0) {
            totalAlpha += _CSTrait::nativeArray(*colors)[0] * weight;
        }
        ++colors;
        ++kernelValues;
    }

    if (channelFlags.isEmpty() || channelFlags.testBit(0)) {
        _CSTrait::nativeArray(dst)[0] = _Maths::clamp((totalAlpha / factor) + offset);
    }
}

template <class _CSTrait>
QImage KoAlphaColorSpaceImpl<_CSTrait>::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                          const KoColorProfile *  /*dstProfile*/,
                                          KoColorConversionTransformation::Intent /*renderingIntent*/,
                                          KoColorConversionTransformation::ConversionFlags /*conversionFlags*/) const
{
    const channels_type *srcPtr = _CSTrait::nativeArray(data);

    QImage img(width, height, QImage::Format_Indexed8);
    QVector<QRgb> table;
    for (int i = 0; i < 256; ++i) table.append(qRgb(i, i, i));
    img.setColorTable(table);

    quint8* data_img;
    for (int i = 0; i < height; ++i) {
        data_img = img.scanLine(i);

        for (int j = 0; j < width; ++j) {
            data_img[j] = _MathsToU8::scaleToA(*(srcPtr++));
        }
    }

    return img;
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::toLabA16(const quint8 *src, quint8 *dst, quint32 nPixels) const {
    const channels_type* srcPtr = _CSTrait::nativeArray(src);
    quint16* dstPtr = reinterpret_cast<quint16*>(dst);

    while (nPixels--) {
        dstPtr[0] = KoColorSpaceMaths<channels_type, quint16>::scaleToA(srcPtr[0]);
        dstPtr[1] = UINT16_MAX / 2;
        dstPtr[2] = UINT16_MAX / 2;
        dstPtr[3] = UINT16_MAX;

        srcPtr++;
        dstPtr += 4;
    }
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::fromLabA16(const quint8 *src, quint8 *dst, quint32 nPixels) const {
    const quint16* srcPtr = reinterpret_cast<const quint16*>(src);
    channels_type* dstPtr = _CSTrait::nativeArray(dst);

    while (nPixels--) {
        dstPtr[0] = KoColorSpaceMaths<quint16, channels_type>::scaleToA(UINT16_MULT(srcPtr[0], srcPtr[3]));

        dstPtr++;
        srcPtr += 4;
    }
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::toRgbA16(const quint8 *src, quint8 *dst, quint32 nPixels) const {
    const channels_type* srcPtr = _CSTrait::nativeArray(src);
    quint16* dstPtr = reinterpret_cast<quint16*>(dst);

    while (nPixels--) {
        const quint16 gray = KoColorSpaceMaths<channels_type, quint16>::scaleToA(srcPtr[0]);

        dstPtr[0] = gray;
        dstPtr[1] = gray;
        dstPtr[2] = gray;
        dstPtr[3] = UINT16_MAX;

        srcPtr++;
        dstPtr += 4;
    }
}

template <class _CSTrait>
void KoAlphaColorSpaceImpl<_CSTrait>::fromRgbA16(const quint8 *src, quint8 *dst, quint32 nPixels) const {
    const quint16* srcPtr = reinterpret_cast<const quint16*>(src);
    channels_type* dstPtr = _CSTrait::nativeArray(dst);

    while (nPixels--) {
        // WARNING: we consider red channel only!
        dstPtr[0] = KoColorSpaceMaths<quint16, channels_type>::scaleToA(UINT16_MULT(srcPtr[0], srcPtr[3]));

        dstPtr++;
        srcPtr += 4;
    }
}

template <class _CSTrait>
KoColorSpace* KoAlphaColorSpaceImpl<_CSTrait>::clone() const
{
    return new KoAlphaColorSpaceImpl<_CSTrait>();
}

template <class _CSTrait>
bool KoAlphaColorSpaceImpl<_CSTrait>::preferCompositionInSourceColorSpace() const
{
    return true;
}

template class KoAlphaColorSpaceImpl<AlphaU8Traits>;
template class KoAlphaColorSpaceImpl<AlphaU16Traits>;
#ifdef HAVE_OPENEXR
template class KoAlphaColorSpaceImpl<AlphaF16Traits>;
#endif
template class KoAlphaColorSpaceImpl<AlphaF32Traits>;

/*********************************************************************************************/
/*              KoAlphaColorSpaceFactoryImpl                                                  */
/*********************************************************************************************/

#include <KoColorConversionAlphaTransformation.h>

template <class _CSTrait>
QList<KoColorConversionTransformationFactory *> KoAlphaColorSpaceFactoryImpl<_CSTrait>::colorConversionLinks() const
{
    QList<KoColorConversionTransformationFactory*> factories;

    factories << new KoColorConversionFromAlphaTransformationFactoryImpl<channels_type>(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc");
    factories << new KoColorConversionToAlphaTransformationFactoryImpl<channels_type>(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc");

    factories << new KoColorConversionFromAlphaTransformationFactoryImpl<channels_type>(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), "default");
    factories << new KoColorConversionToAlphaTransformationFactoryImpl<channels_type>(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), "default");

    factories << new KoColorConversionFromAlphaTransformationFactoryImpl<channels_type>(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), "Lab identity built-in");
    factories << new KoColorConversionToAlphaTransformationFactoryImpl<channels_type>(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), "Lab identity built-in");

    return factories;
}

template class KoAlphaColorSpaceFactoryImpl<AlphaU8Traits>;
template class KoAlphaColorSpaceFactoryImpl<AlphaU16Traits>;
#ifdef HAVE_OPENEXR
template class KoAlphaColorSpaceFactoryImpl<AlphaF16Traits>;
#endif
template class KoAlphaColorSpaceFactoryImpl<AlphaF32Traits>;
