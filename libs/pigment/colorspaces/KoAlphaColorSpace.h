/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
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
#include <QBitArray>

#include "DebugPigment.h"
#include "pigment_export.h"

#include "KoColorSpaceAbstract.h"
#include "KoColorSpaceTraits.h"

#include "KoColorModelStandardIds.h"
#include "KoSimpleColorSpaceFactory.h"

typedef KoColorSpaceTrait<quint8, 1, 0> AlphaU8Traits;

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a color common to the mask. The default color is white.
 */
class PIGMENTCMS_EXPORT KoAlphaColorSpace : public KoColorSpaceAbstract<AlphaU8Traits>
{

public:

    KoAlphaColorSpace();

    virtual ~KoAlphaColorSpace();

    virtual KoID colorModelId() const {
        return AlphaColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
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

    virtual KoColorProfile* profile() {
        return m_profile;
    }

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  dstProfile, KoColorConversionTransformation::Intent renderingIntent) const;

    virtual void toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const {
        quint16* lab = reinterpret_cast<quint16*>(dst);
        while (nPixels--) {
            lab[3] = src[0];
            src++;
            lab += 4;
        }
    }
    virtual void fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const {
        const quint16* lab = reinterpret_cast<const quint16*>(src);
        while (nPixels--) {
            dst[0] = lab[3];
            dst++;
            lab += 4;
        }
    }

    virtual void toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const {
        quint16* rgb = reinterpret_cast<quint16*>(dst);
        while (nPixels--) {
            rgb[3] = src[0];
            src++;
            rgb += 4;
        }
    }
    virtual void fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const {
        const quint16* rgb = reinterpret_cast<const quint16*>(src);
        while (nPixels--) {
            dst[0] = rgb[3];
            dst++;
            rgb += 4;
        }
    }
    virtual KoColorTransformation* createBrightnessContrastAdjustment(const quint16* transferValues) const {
        Q_UNUSED(transferValues);
        warnPigment << i18n("Undefined operation in the alpha color space");
        return 0;
    }
    virtual KoColorTransformation* createDesaturateAdjustment() const {
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

public:

    /**
     * Convert a byte array of srcLen pixels *src to the specified color space
     * and put the converted bytes into the prepared byte array *dst.
     *
     * Returns false if the conversion failed, true if it succeeded
     */
    virtual bool convertPixelsTo(const quint8 *src,
                                 quint8 *dst, const KoColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 KoColorConversionTransformation::Intent  renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
private:
    KoColorProfile* m_profile;
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

    virtual KoColorSpace *createColorSpace(const KoColorProfile *) const {
        return new KoAlphaColorSpace();
    }

};

#endif // KO_COLORSPACE_ALPHA_H_
