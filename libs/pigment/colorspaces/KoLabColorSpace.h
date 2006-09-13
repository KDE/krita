/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KOLABCOLORSPACETRAIT_H
#define KOLABCOLORSPACETRAIT_H

#include <QColor>

#include <klocale.h>
#include <koffice_export.h>

#include <KoIntegerMaths.h>
#include <KoU16ColorSpaceTrait.h>
#include <KoLcmsColorSpaceTrait.h>

class PIGMENT_EXPORT KoLabColorSpace : public KoU16ColorSpaceTrait, public KoLcmsColorSpaceTrait {
public:
    KoLabColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
    virtual ~KoLabColorSpace();

public:

    /**
     * simply COPY the provided data. This method is provided to provide consistency,
     * but you really don't want to be calling it.
     */
    virtual void toLabA16(const quint8 *src, quint8 * dst, const quint32 nPixels) const;

    /**
     * simply COPY the provided data. This method is provided for consistency,
     * but you really don't want to call it.
     */
    virtual void fromLabA16(const quint8 *src, quint8 * dst, const quint32 nPixels) const;



    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8) 
                return true;
            else
                return false;
        };

    virtual Q3ValueVector<KoChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 pixelSize() const;

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual void getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex);

    virtual quint8 difference(const quint8 *src1, const quint8 *src2);
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;
    virtual void invertColor(quint8 * src, qint32 nPixels);
    virtual void convolveColors(quint8** colors, qint32 * kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const;
    
    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const;

protected:

private:

    static const quint32 NUM_CHANNELS = 4;
    static const quint32 NUM_COLOR_CHANNELS = 3;

    static const quint32 CHANNEL_L = 0;
    static const quint32 CHANNEL_A = 1;
    static const quint32 CHANNEL_B = 2;
    static const quint32 CHANNEL_ALPHA = 3;

    static const quint32 MAX_CHANNEL_L = 0xff00;
    static const quint32 MAX_CHANNEL_AB = 0xffff;
    static const quint32 CHANNEL_AB_ZERO_OFFSET = 0x8000;

    friend class KoLabColorSpaceTester;
};

class KoLabColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("LABA", i18n("L*a*b* (16-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return (COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1)); };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigLabData; };
    
    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KoLabColorSpace(parent, p); };

    virtual QString defaultProfile() { return "Lab built-in - (lcms internal)"; };
};

#endif // KOLABCOLORSPACETRAIT_H
