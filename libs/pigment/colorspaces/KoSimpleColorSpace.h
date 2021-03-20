/*
 *  SPDX-FileCopyrightText: 2004-2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOSIMPLECOLORSPACE_H
#define KOSIMPLECOLORSPACE_H

#include <QColor>

#include "DebugPigment.h"

#include "KoColorSpaceAbstract.h"
#include "KoSimpleColorSpaceFactory.h"
#include "KoColorModelStandardIds.h"
#include "colorprofiles/KoDummyColorProfile.h"

template<class _CSTraits>
class KoSimpleColorSpace : public KoColorSpaceAbstract<_CSTraits>
{

public:

    KoSimpleColorSpace(const QString& id,
                       const QString& name,
                       const KoID& colorModelId,
                       const KoID& colorDepthId)
            : KoColorSpaceAbstract<_CSTraits>(id, name)
            , m_name(name)
            , m_colorModelId(colorModelId)
            , m_colorDepthId(colorDepthId)
            , m_profile(new KoDummyColorProfile) {
    }

    ~KoSimpleColorSpace() override {
        delete m_profile;
    }

    KoID colorModelId() const override {
        return m_colorModelId;
    }

    KoID colorDepthId() const override {
        return m_colorDepthId;
    }

    bool willDegrade(ColorSpaceIndependence independence) const override {
        Q_UNUSED(independence);
        return false;
    }

    bool profileIsCompatible(const KoColorProfile* /*profile*/) const override {
        return true;
    }

    quint8 difference(const quint8 *src1, const quint8 *src2) const override {
        Q_UNUSED(src1);
        Q_UNUSED(src2);
        warnPigment << i18n("Undefined operation in the %1 space", m_name);
        return 0;
    }

    quint8 differenceA(const quint8 *src1, const quint8 *src2) const override {
        Q_UNUSED(src1);
        Q_UNUSED(src2);
        warnPigment << i18n("Undefined operation in the %1 space", m_name);
        return 0;
    }

    virtual quint32 colorSpaceType() const {
        return 0;
    }

    bool hasHighDynamicRange() const override {
        return false;
    }

    const KoColorProfile* profile() const override {
        return m_profile;
    }

    KoColorTransformation* createBrightnessContrastAdjustment(const quint16*) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
        return 0;
    }

    virtual KoColorTransformation* createDesaturateAdjustment() const {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
        return 0;
    }

    KoColorTransformation* createPerChannelAdjustment(const quint16* const*) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
        return 0;
    }

    KoColorTransformation *createDarkenAdjustment(qint32 , bool , qreal) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
        return 0;
    }

    virtual void invertColor(quint8*, qint32) const {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
    }

    void colorToXML(const quint8* , QDomDocument& , QDomElement&) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
    }

    void colorFromXML(quint8* , const QDomElement&) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
    }
    void toHSY(const QVector<double> &, qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
    }
    QVector <double> fromHSY(qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
        QVector <double> channelValues (2);
        channelValues.fill(0.0);
        return channelValues;
    }
    void toYUV(const QVector<double> &, qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
    }
    QVector <double> fromYUV(qreal *, qreal *, qreal *) const override {
        warnPigment << i18n("Undefined operation in the %1 color space", m_name);
        QVector <double> channelValues (2);
        channelValues.fill(0.0);
        return channelValues;
    }

    void toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == LABAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        } else {
            const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->lab16();
            convertPixelsTo(src, dst, dstCs, nPixels,
                            KoColorConversionTransformation::internalRenderingIntent(),
                            KoColorConversionTransformation::internalConversionFlags());
        }
    }

    void fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == LABAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        } else {
            const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->lab16();
            srcCs->convertPixelsTo(src, dst, this, nPixels,
                                   KoColorConversionTransformation::internalRenderingIntent(),
                                   KoColorConversionTransformation::internalConversionFlags());
        }
    }

    void toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == RGBAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        } else {
            const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->rgb16();
            convertPixelsTo(src, dst, dstCs, nPixels,
                            KoColorConversionTransformation::internalRenderingIntent(),
                            KoColorConversionTransformation::internalConversionFlags());
        }
    }

    void fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == RGBAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        } else {
            const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->rgb16();
            srcCs->convertPixelsTo(src, dst, this, nPixels,
                                   KoColorConversionTransformation::internalRenderingIntent(),
                                   KoColorConversionTransformation::internalConversionFlags());
        }
    }

    bool convertPixelsTo(const quint8 *src,
                                 quint8 *dst, const KoColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 KoColorConversionTransformation::Intent renderingIntent,
                                 KoColorConversionTransformation::ConversionFlags conversionFlags) const override
    {
        Q_UNUSED(renderingIntent);
        Q_UNUSED(conversionFlags);

        QColor c;
        quint32 srcPixelsize = this->pixelSize();
        quint32 dstPixelsize = dstColorSpace->pixelSize();

        while (numPixels > 0) {

            this->toQColor(src, &c);
            dstColorSpace->fromQColor(c, dst);

            src += srcPixelsize;
            dst += dstPixelsize;

            --numPixels;
        }
        return true;
    }


    virtual QString colorSpaceEngine() const {
        return "simple";
    }

private:
    QString m_name;
    KoID m_colorModelId;
    KoID m_colorDepthId;
    KoColorProfile* m_profile;

};


#endif // KOSIMPLECOLORSPACE_H
