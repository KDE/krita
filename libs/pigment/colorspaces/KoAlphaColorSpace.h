/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KOALPHACOLORSPACE_H
#define KOALPHACOLORSPACE_H

#include <QColor>

#include "DebugPigment.h"
#include "kritapigment_export.h"

#include "KoColorSpaceAbstract.h"
#include "KoColorSpaceTraits.h"

#include "KoColorModelStandardIds.h"
#include "KoColorModelStandardIdsUtils.h"
#include "KoSimpleColorSpaceFactory.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif


template <typename _T, typename _Tdst>
class KoColorSpaceMaths;

typedef KoColorSpaceTrait<quint8, 1, 0> AlphaU8Traits;
typedef KoColorSpaceTrait<quint16, 1, 0> AlphaU16Traits;
typedef KoColorSpaceTrait<float, 1, 0> AlphaF32Traits;

template <typename channel_type> KoID alphaIdFromChannelType();
template <> inline KoID alphaIdFromChannelType<quint8>() { return KoID("ALPHA", i18n("Alpha (8-bit integer)")); }
template <> inline KoID alphaIdFromChannelType<quint16>() { return KoID("ALPHAU16", i18n("Alpha (16-bit integer)")); }
template <> inline KoID alphaIdFromChannelType<float>() { return KoID("ALPHAF32", i18n("Alpha (32-bit floating point)")); }

#ifdef HAVE_OPENEXR
typedef KoColorSpaceTrait<half, 1, 0> AlphaF16Traits;
template <> inline KoID alphaIdFromChannelType<half>() { return KoID("ALPHAF16", i18n("Alpha (16-bit floating point)")); }
#endif

class QBitArray;

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a color common to the mask. The default color is white.
 */
template <class _CSTrait>
class KRITAPIGMENT_EXPORT KoAlphaColorSpaceImpl : public KoColorSpaceAbstract<_CSTrait>
{
    typedef typename _CSTrait::channels_type channels_type;
    typedef KoColorSpaceMaths<channels_type> _Maths;
    typedef KoColorSpaceMaths<channels_type, quint8> _MathsToU8;
    typedef KoColorSpaceMaths<quint8, channels_type> _MathsFromU8;


public:

    KoAlphaColorSpaceImpl();

    virtual ~KoAlphaColorSpaceImpl();

    static QString colorSpaceId() {
        return alphaIdFromChannelType<channels_type>().id();
    }

    virtual KoID colorModelId() const {
        return AlphaColorModelID;
    }

    virtual KoID colorDepthId() const {
        return colorDepthIdForChannelType<channels_type>();
    }

    virtual KoColorSpace* clone() const;

    virtual bool willDegrade(ColorSpaceIndependence independence) const {
        Q_UNUSED(independence);
        return false;
    }

    virtual bool profileIsCompatible(const KoColorProfile* /*profile*/) const {
        return false;
    }

    virtual void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const;

    virtual void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const;

    virtual quint8 difference(const quint8 *src1, const quint8 *src2) const;
    virtual quint8 differenceA(const quint8 *src1, const quint8 *src2) const;

    virtual quint32 colorChannelCount() const {
        return 0;
    }

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual void convolveColors(quint8** colors, qreal* kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nColors, const QBitArray & channelFlags) const;

    virtual quint32 colorSpaceType() const {
        return 0;
    }

    virtual bool hasHighDynamicRange() const {
        return false;
    }

    virtual const KoColorProfile* profile() const {
        return m_profile;
    }

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  dstProfile,
                                   KoColorConversionTransformation::Intent renderingIntent,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags) const;

    virtual void toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const;
    virtual void fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const;

    virtual void toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const;
    virtual void fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const;

    virtual KoColorTransformation* createBrightnessContrastAdjustment(const quint16* transferValues) const {
        Q_UNUSED(transferValues);
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }

    virtual KoColorTransformation* createPerChannelAdjustment(const quint16* const*) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }

    virtual KoColorTransformation *createDarkenAdjustment(qint32 , bool , qreal) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }

    virtual void invertColor(quint8*, qint32) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    virtual void colorToXML(const quint8* , QDomDocument& , QDomElement&) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    virtual void colorFromXML(quint8* , const QDomElement&) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    virtual void toHSY(const QVector<double> &, qreal *, qreal *, qreal *) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    virtual QVector <double> fromHSY(qreal *, qreal *, qreal *) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
        QVector <double> channelValues (1);
        channelValues.fill(0.0);
        return channelValues;
    }

    virtual void toYUV(const QVector<double> &, qreal *, qreal *, qreal *) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    virtual QVector <double> fromYUV(qreal *, qreal *, qreal *) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
        QVector <double> channelValues (1);
        channelValues.fill(0.0);
        return channelValues;
    }

protected:
    virtual bool preferCompositionInSourceColorSpace() const;

private:
    KoColorProfile* m_profile;
    QList<KoCompositeOp*> m_compositeOps;
};

typedef KoAlphaColorSpaceImpl<AlphaU8Traits> KoAlphaColorSpace;
typedef KoAlphaColorSpaceImpl<AlphaU16Traits> KoAlphaU16ColorSpace;
#ifdef HAVE_OPENEXR
typedef KoAlphaColorSpaceImpl<AlphaF16Traits> KoAlphaF16ColorSpace;
#endif
typedef KoAlphaColorSpaceImpl<AlphaF32Traits> KoAlphaF32ColorSpace;

template <class _CSTrait>
class KoAlphaColorSpaceFactoryImpl : public KoSimpleColorSpaceFactory
{
    typedef typename _CSTrait::channels_type channels_type;

public:
    KoAlphaColorSpaceFactoryImpl()
            : KoSimpleColorSpaceFactory(alphaIdFromChannelType<channels_type>().id(),
                                        alphaIdFromChannelType<channels_type>().name(),
                                        true,
                                        AlphaColorModelID,
                                        colorDepthIdForChannelType<channels_type>(),
                                        qMin(16, int(sizeof(channels_type) * 8)), // DIRTY HACK ALERT: see a comment below!
                                        100000)
    {
        /**
         * Note about a hack with reference bit depth: right now all the color
         * conversions to/from Alpha colorspace are done via LabAU16 16-bit color space,
         * therefore, the conversions are lossy! Better conversions are yet to be implemented,
         * but for now we just define this color space as having 16-bit reference depth
         * (the problem arises with AlphaF32 only of course, which is hardly used anywhere).
         */
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *) const {
        return new KoAlphaColorSpaceImpl<_CSTrait>();
    }

    virtual QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;
};

typedef KoAlphaColorSpaceFactoryImpl<AlphaU8Traits> KoAlphaColorSpaceFactory;
typedef KoAlphaColorSpaceFactoryImpl<AlphaU16Traits> KoAlphaU16ColorSpaceFactory;
#ifdef HAVE_OPENEXR
typedef KoAlphaColorSpaceFactoryImpl<AlphaF16Traits> KoAlphaF16ColorSpaceFactory;
#endif
typedef KoAlphaColorSpaceFactoryImpl<AlphaF32Traits> KoAlphaF32ColorSpaceFactory;

#endif
