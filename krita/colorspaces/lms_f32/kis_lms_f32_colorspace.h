/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_STRATEGY_COLORSPACE_LMS_F32_H_
#define KIS_STRATEGY_COLORSPACE_LMS_F32_H_

#include <QColor>

#include <klocale.h>

#include <krita_export.h>

#include "KoF32HalfColorSpaceTrait.h"
#include "KoLcmsColorSpaceTrait.h"

class KoColorSpaceRegistry;

class KRITACOLOR_EXPORT KisLmsF32ColorSpace : public KoF32ColorSpaceTrait, public KoLcmsColorSpaceTrait {
public:
    KisLmsF32ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
    virtual ~KisLmsF32ColorSpace();

    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8 || independence == TO_LAB16)
                return true;
            else
                return false;
        };


public:
    void setPixel(quint8 *pixel, float longWave, float middleWave, float shortWave, float alpha) const;
    void getPixel(const quint8 *pixel, float *longWave, float *middleWave, float *shortWave, float *alpha) const;

    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile * profile = 0);

    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile = 0);

    virtual quint8 difference(const quint8 *src1, const quint8 *src2);
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;

    virtual Q3ValueVector<KoChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 pixelSize() const;

    virtual bool hasHighDynamicRange() const { return false; }

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                       KoColorProfile *  dstProfile,
                       qint32 renderingIntent,
                       float exposure = 0.0f);

    virtual KoCompositeOpList userVisiblecompositeOps() const;


protected:

    virtual void bitBlt(quint8 *dst,
                qint32 dstRowStride,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KoCompositeOp* op);

    void compositeOver(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeErase(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);
    void compositeCopy(quint8 *dst, qint32 dstRowStride, const quint8 *src, qint32 srcRowStride, const quint8 *mask, qint32 maskRowStride, qint32 rows, qint32 columns, float opacity);

private:
    inline quint8 computeRed(float l, float m, float s) const
    {
        return FLOAT_TO_UINT8(4.4679*l - 3.58738*m + 0.1193*s);
    }
    inline quint8 computeGreen(float l, float m, float s) const
    {
        return FLOAT_TO_UINT8(-1.2186*l + 2.3809*m - 0.1624*s);
    }
    inline quint8 computeBlue(float l, float m, float s) const
    {
        return FLOAT_TO_UINT8(0.0497*l - 0.2439*m + 1.2045*s);
    }
    inline float computeLong(quint8 r, quint8 g, quint8 b) const
    {
        return 0.3811*UINT8_TO_FLOAT(r) + 0.5783*UINT8_TO_FLOAT(g) + 0.0402*UINT8_TO_FLOAT(b);
    }
    inline float computeMiddle(quint8 r, quint8 g, quint8 b) const
    {
        return 0.1967*UINT8_TO_FLOAT(r) + 0.7244*UINT8_TO_FLOAT(g) + 0.0782*UINT8_TO_FLOAT(b);
    }
    inline float computeShort(quint8 r, quint8 g, quint8 b) const
    {
        return 0.0241*UINT8_TO_FLOAT(r) + 0.1288*UINT8_TO_FLOAT(g) + 0.8444*UINT8_TO_FLOAT(b);
    }

    friend class KisLmsF32ColorSpaceTester;

    static const quint8 PIXEL_LONGWAVE = 0;
    static const quint8 PIXEL_MIDDLEWAVE = 1;
    static const quint8 PIXEL_SHORTWAVE = 2;
    static const quint8 PIXEL_ALPHA = 3;

    struct Pixel {
        float longWave;
        float middleWave;
        float shortWave;
        float alpha;
    };
};

class KisLmsF32ColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("LMSAF32", i18n("LMS Cone Space (32-bit float/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return 0; }; // FIXME: lcms do not support LMS cone space

    virtual icColorSpaceSignature colorSpaceSignature() { return icMaxEnumData; };

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KisLmsF32ColorSpace(parent, p); };

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_LMS_F32_H_

