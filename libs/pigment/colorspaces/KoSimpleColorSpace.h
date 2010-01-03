/*
 *  Copyright (c) 2004-2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KOSIMPLECOLORSPACE_H
#define KOSIMPLECOLORSPACE_H

#include <QColor>
#include <QBitArray>

#include "DebugPigment.h"

#include "KoColorSpaceAbstract.h"
#include "KoColorSpaceTraits.h"
#include "KoSimpleColorSpaceFactory.h"
#include "KoColorModelStandardIds.h"

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
            , m_colorDepthId(colorDepthId) {
    }

    virtual ~KoSimpleColorSpace() {
    }

    virtual KoID colorModelId() const {
        return m_colorModelId;
    }

    virtual KoID colorDepthId() const {
        return m_colorDepthId;
    }

    virtual bool willDegrade(ColorSpaceIndependence independence) const {
        Q_UNUSED(independence);
        return false;
    }

    virtual bool profileIsCompatible(const KoColorProfile* /*profile*/) const {
        return true;
    }

    virtual quint8 difference(const quint8 *src1, const quint8 *src2) const {
        Q_UNUSED(src1);
        Q_UNUSED(src2);
        warnPigment << i18n("Undefined operation in the %1 space").arg(m_name);
        return 0;
    }

    virtual quint32 colorSpaceType() const {
        return 0;
    }

    virtual bool hasHighDynamicRange() const {
        return false;
    }

    virtual const KoColorProfile* profile() const {
        return 0;
    }

    virtual KoColorProfile* profile() {
        return 0;
    }

    virtual KoColorTransformation* createBrightnessContrastAdjustment(const quint16*) const {
        warnPigment << i18n("Undefined operation in the %1 space").arg(m_name);
        return 0;
    }

    virtual KoColorTransformation* createDesaturateAdjustment() const {
        warnPigment << i18n("Undefined operation in the %1 color space").arg(m_name);
        return 0;
    }

    virtual KoColorTransformation* createPerChannelAdjustment(const quint16* const*) const {
        warnPigment << i18n("Undefined operation in the %1 color space").arg(m_name);
        return 0;
    }

    virtual KoColorTransformation *createDarkenAdjustment(qint32 , bool , qreal) const {
        warnPigment << i18n("Undefined operation in the %1 color space").arg(m_name);
        return 0;
    }

    virtual void invertColor(quint8*, qint32) const {
        warnPigment << i18n("Undefined operation in the %1 color space").arg(m_name);
    }

    virtual void colorToXML(const quint8* , QDomDocument& , QDomElement&) const {
        warnPigment << i18n("Undefined operation in the %1 color space").arg(m_name);
    }

    virtual void colorFromXML(quint8* , const QDomElement&) const {
        warnPigment << i18n("Undefined operation in the %1 color space").arg(m_name);
    }

    virtual void toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const
    {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == LABAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        }
        else {
            const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->lab16();
            convertPixelsTo(src, dst, dstCs, nPixels);
        }
    }

    virtual void fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const
    {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == LABAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        }
        else {
            const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->lab16();
            srcCs->convertPixelsTo(src, dst, this, nPixels);
        }
    }

    virtual void toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const
    {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == RGBAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        }
        else {
            const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->rgb16();
            convertPixelsTo(src, dst, dstCs, nPixels);
        }
    }

    virtual void fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const
    {
        if (colorDepthId() == Integer16BitsColorDepthID && colorModelId() == RGBAColorModelID) {
            memcpy(dst, src, nPixels * 2);
        }
        else {
            const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->rgb16();
            srcCs->convertPixelsTo(src, dst, this, nPixels);
        }
    }

    virtual bool convertPixelsTo(const quint8 *src,
                                 quint8 *dst, const KoColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 KoColorConversionTransformation::Intent  renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
    {
        Q_UNUSED(renderingIntent);

        QColor c;
        quint32 srcPixelsize = this->pixelSize();
        quint32 dstPixelsize = dstColorSpace->pixelSize();

        while(numPixels > 0) {

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

};


#endif // KOSIMPLECOLORSPACE_H
