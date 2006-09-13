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

#ifndef KIS_YCBCR_U8_COLORSPACE_H
#define KIS_YCBCR_U8_COLORSPACE_H

#include <KoU8ColorSpaceTrait.h>

#include <klocale.h>

#define LUMA_RED 0.2989
#define LUMA_GREEN 0.587
#define LUMA_BLUE 0.114

class KisYCbCrU8ColorSpace : public KoU8ColorSpaceTrait
{
    public:
        KisYCbCrU8ColorSpace(KoColorSpaceRegistry* parent, KoColorProfile* p);
        ~KisYCbCrU8ColorSpace();
        virtual bool willDegrade(ColorSpaceIndependence )
        {
            return false;
        };
    public:
        void setPixel(Q_UINT8 *pixel, Q_UINT8 Y, Q_UINT8 Cb, Q_UINT8 Cr, Q_UINT8 alpha) const;
        void getPixel(const Q_UINT8 *pixel, Q_UINT8 *Y, Q_UINT8 *Cb, Q_UINT8 *Cr, Q_UINT8 *alpha) const;

        virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KoColorProfile * profile = 0);
        virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KoColorProfile * profile = 0);

        virtual void toQColor(const Q_UINT8 *src, QColor *c, KoColorProfile * profile = 0);
        virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KoColorProfile * profile = 0);

        virtual Q_UINT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
        virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

        virtual QValueVector<KoChannelInfo *> channels() const;
        virtual Q_UINT32 nChannels() const;
        virtual Q_UINT32 nColorChannels() const;
        virtual Q_UINT32 pixelSize() const;

        virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                                       KoColorProfile *  dstProfile,
                                       Q_INT32 renderingIntent,
                                       float exposure = 0.0f);

        virtual KoCompositeOpList userVisiblecompositeOps() const;

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
                            const KoCompositeOp* op);

        void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
        void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);
        void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity);

    private:
#define CLAMP_TO_8BITCHANNEL(a) CLAMP(a, 0, Q_UINT8_MAX)
        inline Q_UINT8 computeRed(Q_UINT8 Y, Q_UINT8 /*Cb*/, Q_UINT8 Cr)
        {
            return (Q_UINT8)( CLAMP_TO_8BITCHANNEL( (Cr - 128)* (2-2*LUMA_RED) + Y )  );
        }
        inline Q_UINT8 computeGreen(Q_UINT8 Y, Q_UINT8 Cb, Q_UINT8 Cr)
        {
            return (Q_UINT8)( CLAMP_TO_8BITCHANNEL( (Y - LUMA_BLUE * computeBlue(Y,Cb,Cr) - LUMA_RED * computeRed(Y,Cb,Cr) ) / LUMA_GREEN ) );
        }
        inline Q_UINT8 computeBlue(Q_UINT8 Y, Q_UINT8 Cb, Q_UINT8 /*Cr*/)
        {
            return (Q_UINT8)( CLAMP_TO_8BITCHANNEL( (Cb - 128)*(2 - 2 * LUMA_BLUE) + Y) );
        }
        inline Q_UINT8 computeY( Q_UINT8 r, Q_UINT8 b, Q_UINT8 g)
        {
            return (Q_UINT8)( CLAMP_TO_8BITCHANNEL( LUMA_RED*r + LUMA_GREEN*g + LUMA_BLUE*b ) );
        }
        inline Q_UINT8 computeCb( Q_UINT8 r, Q_UINT8 b, Q_UINT8 g)
        {
            return (Q_UINT8)( CLAMP_TO_8BITCHANNEL( (b - computeY(r,g,b))/(2-2*LUMA_BLUE) + 128) );
        }
        inline Q_UINT8 computeCr( Q_UINT8 r, Q_UINT8 b, Q_UINT8 g)
        {
            return (Q_UINT8)( CLAMP_TO_8BITCHANNEL( (r - computeY(r,g,b))/(2-2*LUMA_RED) + 128) );
        }
#undef CLAMP_TO_8BITCHANNEL

        static const Q_UINT8 PIXEL_Y = 0;
        static const Q_UINT8 PIXEL_Cb = 1;
        static const Q_UINT8 PIXEL_Cr = 2;
        static const Q_UINT8 PIXEL_ALPHA = 3;

        struct Pixel {
            Q_UINT8 Y;
            Q_UINT8 Cb;
            Q_UINT8 Cr;
            Q_UINT8 alpha;
        };
};

class KisYCbCrU8ColorSpaceFactory : public KoColorSpaceFactory
{
    public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
        virtual KoID id() const { return KoID("YCbCrAU8", i18n("YCBCR (8-bit integer/channel)")); };

    /**
         * lcms colorspace type definition.
     */
        virtual Q_UINT32 colorSpaceType() { return TYPE_YCbCr_8; };

        virtual icColorSpaceSignature colorSpaceSignature() { return icSigYCbCrData; };

        virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KisYCbCrU8ColorSpace(parent, p); };

        virtual QString defaultProfile() { return ""; };
};

#endif
