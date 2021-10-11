/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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

    ~KoAlphaColorSpaceImpl() override;

    static QString colorSpaceId() {
        return alphaIdFromChannelType<channels_type>().id();
    }

    KoID colorModelId() const override {
        return AlphaColorModelID;
    }

    KoID colorDepthId() const override {
        return colorDepthIdForChannelType<channels_type>();
    }

    virtual KoColorSpace* clone() const;

    bool willDegrade(ColorSpaceIndependence independence) const override {
        Q_UNUSED(independence);
        return false;
    }

    bool profileIsCompatible(const KoColorProfile* /*profile*/) const override {
        return false;
    }

    void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const override;

    void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const override;

    quint8 difference(const quint8 *src1, const quint8 *src2) const override;
    quint8 differenceA(const quint8 *src1, const quint8 *src2) const override;

    quint32 colorChannelCount() const override {
        return 0;
    }

    QString channelValueText(const quint8 *pixel, quint32 channelIndex) const override;

    QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const override;

    virtual void convolveColors(quint8** colors, qreal* kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nColors, const QBitArray & channelFlags) const;

    virtual quint32 colorSpaceType() const {
        return 0;
    }

    bool hasHighDynamicRange() const override {
        return false;
    }

    const KoColorProfile* profile() const override {
        return m_profile;
    }

    QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  dstProfile,
                                   KoColorConversionTransformation::Intent renderingIntent,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags) const override;

    void toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const override;
    void fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const override;

    void toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const override;
    void fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const override;

    KoColorTransformation* createBrightnessContrastAdjustment(const quint16* transferValues) const override {
        Q_UNUSED(transferValues);
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }

    KoColorTransformation* createPerChannelAdjustment(const quint16* const*) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }

    KoColorTransformation *createDarkenAdjustment(qint32 , bool , qreal) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }

    virtual void invertColor(quint8*, qint32) const {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    void colorToXML(const quint8* , QDomDocument& , QDomElement&) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    void colorFromXML(quint8* , const QDomElement&) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    void toHSY(const QVector<double> &, qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    QVector <double> fromHSY(qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
        QVector <double> channelValues (1);
        channelValues.fill(0.0);
        return channelValues;
    }

    void toYUV(const QVector<double> &, qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
    }

    QVector <double> fromYUV(qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the alpha color space");
        QVector <double> channelValues (1);
        channelValues.fill(0.0);
        return channelValues;
    }

protected:
    bool preferCompositionInSourceColorSpace() const override;

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
                                        false,
                                        AlphaColorModelID,
                                        colorDepthIdForChannelType<channels_type>(),
                                        sizeof(channels_type) * 8,
                                        100000)
    {
    }

    KoColorSpace *createColorSpace(const KoColorProfile *) const override {
        return new KoAlphaColorSpaceImpl<_CSTrait>();
    }

    QList<KoColorConversionTransformationFactory*> colorConversionLinks() const override;
};

typedef KoAlphaColorSpaceFactoryImpl<AlphaU8Traits> KoAlphaColorSpaceFactory;
typedef KoAlphaColorSpaceFactoryImpl<AlphaU16Traits> KoAlphaU16ColorSpaceFactory;
#ifdef HAVE_OPENEXR
typedef KoAlphaColorSpaceFactoryImpl<AlphaF16Traits> KoAlphaF16ColorSpaceFactory;
#endif
typedef KoAlphaColorSpaceFactoryImpl<AlphaF32Traits> KoAlphaF32ColorSpaceFactory;

#endif
