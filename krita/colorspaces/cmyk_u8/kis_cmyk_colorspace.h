/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KIS_STRATEGY_COLORSPACE_CMYK_H_
#define KIS_STRATEGY_COLORSPACE_CMYK_H_

#include <qcolor.h>
#include <qmap.h>
#include <koffice_export.h>
#include "kis_global.h"
#include "kis_u8_base_colorspace.h"

class KRITACORE_EXPORT KisCmykColorSpace : public KisU8BaseColorSpace {

public:

        
    struct Pixel {
            Q_UINT16 cyan;
            Q_UINT16 magenta;
            Q_UINT16 yellow;
            Q_UINT16 black;
            Q_UINT16 alpha;
        };
public:
    KisCmykColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisCmykColorSpace();


    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8)
                return true;
            else
                return false;
        };



public:

    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;
    virtual void applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *adj, Q_INT32 nPixels);
    virtual void invertColor(Q_UINT8 * src, Q_INT32 nPixels);
    virtual void convolveColors(Q_UINT8** colors, Q_INT32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const;
    // XXX: darken & intensity8?
    
    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual Q_UINT32 nChannels() const;
    virtual Q_UINT32 nColorChannels() const;
    virtual Q_UINT32 pixelSize() const;
    virtual void getSingleChannelPixel(Q_UINT8 *dstPixel, const Q_UINT8 *srcPixel, Q_UINT32 channelIndex);
    
    virtual KisCompositeOpList userVisiblecompositeOps() const;

protected:
    
    virtual void bitBlt(Q_UINT8 *dst,
                        Q_INT32 dstRowStride,
                        const Q_UINT8 *src,
                        Q_INT32 srcRowStride,
                        const Q_UINT8 *srcAlphaMask,
                        Q_INT32 maskRowStride,
                        Q_UINT8 opacity,
                        Q_INT32 rows,
                        Q_INT32 cols,
                        const KisCompositeOp& op);

    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeMultiply(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeDivide(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeScreen(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeOverlay(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeDodge(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeBurn(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeDarken(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeLighten(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
    void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);

private:

    Q_UINT8 * m_qcolordata;

    static const Q_UINT8 PIXEL_CYAN = 0;
    static const Q_UINT8 PIXEL_MAGENTA = 1;
    static const Q_UINT8 PIXEL_YELLOW = 2;
    static const Q_UINT8 PIXEL_BLACK = 3;
    static const Q_UINT8 PIXEL_CMYK_ALPHA = 4;
};

class KisCmykColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("CMYK", i18n("CMYK (8-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual Q_UINT32 colorSpaceType() { return TYPE_CMYK5_8; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigCmykData; };

    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisCmykColorSpace(parent, p); };

    virtual QString defaultProfile() { return "Adobe CMYK"; }; //  Do not i18n -- this is from a data file
};

#endif // KIS_STRATEGY_COLORSPACE_CMYK_H_
