/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include "KoSimpleColorSpaceFactory.h"

typedef KoColorSpaceTrait<quint8, 1, 0> AlphaU8Traits;

class QBitArray;

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a color common to the mask. The default color is white.
 */
class KRITAPIGMENT_EXPORT KoAlphaColorSpace : public KoColorSpaceAbstract<AlphaU8Traits>
{

public:

    KoAlphaColorSpace();

    ~KoAlphaColorSpace() override;

    static QString colorSpaceId() { return "ALPHA"; }

    KoID colorModelId() const override {
        return AlphaColorModelID;
    }

    KoID colorDepthId() const override {
        return Integer8BitsColorDepthID;
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

    void toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        quint16* lab = reinterpret_cast<quint16*>(dst);
        while (nPixels--) {
            lab[3] = src[0];
            src++;
            lab += 4;
        }
    }
    void fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        const quint16* lab = reinterpret_cast<const quint16*>(src);
        while (nPixels--) {
            dst[0] = lab[3];
            dst++;
            lab += 4;
        }
    }

    void toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        quint16* rgb = reinterpret_cast<quint16*>(dst);
        while (nPixels--) {
            rgb[3] = src[0];
            src++;
            rgb += 4;
        }
    }
    void fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const override {
        const quint16* rgb = reinterpret_cast<const quint16*>(src);
        while (nPixels--) {
            dst[0] = rgb[3];
            dst++;
            rgb += 4;
        }
    }
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


class KoAlphaColorSpaceFactory : public KoSimpleColorSpaceFactory
{

public:
    KoAlphaColorSpaceFactory()
            : KoSimpleColorSpaceFactory("ALPHA",
                                        i18n("Alpha mask"),
                                        false,
                                        AlphaColorModelID,
                                        Integer8BitsColorDepthID,
                                        8) {
    }

    KoColorSpace *createColorSpace(const KoColorProfile *) const override {
        return new KoAlphaColorSpace();
    }

};

#endif // KO_COLORSPACE_ALPHA_H_
