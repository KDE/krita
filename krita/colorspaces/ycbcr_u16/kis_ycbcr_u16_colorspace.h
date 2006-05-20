/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_YCBCR_U16_COLORSPACE_H
#define KIS_YCBCR_U16_COLORSPACE_H

#include <kis_u16_base_colorspace.h>

#include <klocale.h>

#define LUMA_RED 0.2989
#define LUMA_GREEN 0.587
#define LUMA_BLUE 0.114

class KisYCbCrU16ColorSpace : public KisU16BaseColorSpace
{
    public:
        KisYCbCrU16ColorSpace(KisColorSpaceFactoryRegistry* parent, KisProfile* p);
        ~KisYCbCrU16ColorSpace();
        virtual bool willDegrade(ColorSpaceIndependence )
        {
            return false;
        };
    public:
        void setPixel(Q_UINT8 *pixel, Q_UINT16 Y, Q_UINT16 Cb, Q_UINT16 Cr, Q_UINT16 alpha) const;
        void getPixel(const Q_UINT8 *pixel, Q_UINT16 *Y, Q_UINT16 *Cb, Q_UINT16 *Cr, Q_UINT16 *alpha) const;

        virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile * profile = 0);
        virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile * profile = 0);

        virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile * profile = 0);
        virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile * profile = 0);

        virtual Q_UINT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
        virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

        virtual QValueVector<KisChannelInfo *> channels() const;
        virtual Q_UINT32 nChannels() const;
        virtual Q_UINT32 nColorChannels() const;
        virtual Q_UINT32 pixelSize() const;

        virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                                       KisProfile *  dstProfile,
                                       Q_INT32 renderingIntent,
                                       float exposure = 0.0f);

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
        void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);

    private:
#define CLAMP_TO_16BITCHANNEL(a) CLAMP(a, 0, Q_UINT16_MAX)
        inline Q_UINT16 computeRed(Q_UINT16 Y, Q_UINT16 /*Cb*/, Q_UINT16 Cr)
        {
            return (Q_UINT16)( CLAMP_TO_16BITCHANNEL( (Cr - 32768)* (2-2*LUMA_RED) + Y )  );
        }
        inline Q_UINT16 computeGreen(Q_UINT16 Y, Q_UINT16 Cb, Q_UINT16 Cr)
        {
            return (Q_UINT16)( CLAMP_TO_16BITCHANNEL( (Y - LUMA_BLUE * computeBlue(Y,Cb,Cr) - LUMA_RED * computeRed(Y,Cb,Cr) ) / LUMA_GREEN ) );
        }
        inline Q_UINT16 computeBlue(Q_UINT16 Y, Q_UINT16 Cb, Q_UINT16 /*Cr*/)
        {
            return (Q_UINT16)( CLAMP_TO_16BITCHANNEL( (Cb - 32768)*(2 - 2 * LUMA_BLUE) + Y) );
        }
        inline Q_UINT16 computeY( Q_UINT16 r, Q_UINT16 b, Q_UINT16 g)
        {
            return (Q_UINT16)( CLAMP_TO_16BITCHANNEL( LUMA_RED*r + LUMA_GREEN*g + LUMA_BLUE*b ) );
        }
        inline Q_UINT16 computeCb( Q_UINT16 r, Q_UINT16 b, Q_UINT16 g)
        {
            return (Q_UINT16)( CLAMP_TO_16BITCHANNEL( (b - computeY(r,g,b))/(2-2*LUMA_BLUE) + 32768) );
        }
        inline Q_UINT16 computeCr( Q_UINT16 r, Q_UINT16 b, Q_UINT16 g)
        {
            return (Q_UINT8)( CLAMP_TO_16BITCHANNEL( (r - computeY(r,g,b))/(2-2*LUMA_RED) + 32768) );
        }
#undef CLAMP_TO_16BITCHANNEL
        
        static const Q_UINT8 PIXEL_Y = 0;
        static const Q_UINT8 PIXEL_Cb = 1;
        static const Q_UINT8 PIXEL_Cr = 2;
        static const Q_UINT8 PIXEL_ALPHA = 3;

        struct Pixel {
            Q_UINT16 Y;
            Q_UINT16 Cb;
            Q_UINT16 Cr;
            Q_UINT16 alpha;
        };
};

class KisYCbCrU16ColorSpaceFactory : public KisColorSpaceFactory
{
    public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
        virtual KisID id() const { return KisID("YCbCrAU16", i18n("YCBCR (16-bit integer/channel)")); };

    /**
         * lcms colorspace type definition.
     */
        virtual Q_UINT32 colorSpaceType() { return TYPE_YCbCr_16; };

        virtual icColorSpaceSignature colorSpaceSignature() { return icSigYCbCrData; };

        virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisYCbCrU16ColorSpace(parent, p); };

        virtual QString defaultProfile() { return ""; };
};


#endif
