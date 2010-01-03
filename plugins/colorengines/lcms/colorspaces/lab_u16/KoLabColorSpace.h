/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KoLabColorSpace_H_
#define KoLabColorSpace_H_

#include "KoLcmsColorSpace.h"
#include "KoColorSpaceTraits.h"
#include "KoColorModelStandardIds.h"

class KoLabColorSpace : public KoLcmsColorSpace<KoLabU16Traits>
{
public:
    KoLabColorSpace(KoColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence independence) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
    virtual KoID colorModelId() const {
        return LABAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer16BitsColorDepthID;
    }
    virtual KoColorSpace* clone() const;
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;

private:
    static const quint32 CHANNEL_L = 0;
    static const quint32 CHANNEL_A = 1;
    static const quint32 CHANNEL_B = 2;
    static const quint32 CHANNEL_ALPHA = 3;
    static const quint32 MAX_CHANNEL_L = 0xff00;
    static const quint32 MAX_CHANNEL_AB = 0xffff;
    static const quint32 CHANNEL_AB_ZERO_OFFSET = 0x8000;
};

class KoLabColorSpaceFactory : public KoLcmsColorSpaceFactory
{
public:
    KoLabColorSpaceFactory() : KoLcmsColorSpaceFactory((COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(2) | EXTRA_SH(1)), icSigLabData) {}
    virtual bool userVisible() const {
        return true;
    }
    /// reimplemented from KoColorSpaceFactory
    virtual QString id() const {
        return KoLabColorSpace::colorSpaceId();
    }
    /// reimplemented from KoColorSpaceFactory
    virtual QString name() const {
        return i18n("L*a*b* (16-bit integer/channel)");
    }

    virtual KoID colorModelId() const {
        return LABAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer16BitsColorDepthID;
    }
    virtual int referenceDepth() const {
        return 16;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new KoLabColorSpace(p->clone());
    }

    virtual QString defaultProfile() const {
        return "Lab built-in - (lcms internal)";
    }
};


#endif
