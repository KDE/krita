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
#include "kis_pixel.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_u8_base_colorspace.h"
#include "kis_abstract_colorspace.h"

class KRITACORE_EXPORT KisCmykColorSpace : public KisU8BaseColorSpace {

public:
    KisCmykColorSpace();
    virtual ~KisCmykColorSpace();

public:

    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfileSP profile = 0)
        { return KisPixelRO (src, src + PIXEL_CMYK_ALPHA, this, profile); }
    virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfileSP profile = 0)
        { return KisPixel (src, src + PIXEL_CMYK_ALPHA, this, profile); }

    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual vKisChannelInfoSP channels() const;
    virtual bool hasAlpha() const;
    virtual Q_INT32 nChannels() const;
    virtual Q_INT32 nColorChannels() const;
    virtual Q_INT32 pixelSize() const;

    virtual void adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const;


virtual void bitBlt(Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src,
            Q_INT32 srcRowStride,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            QUANTUM opacity,
            Q_INT32 rows,
            Q_INT32 cols,
            const KisCompositeOp& op);

    virtual bool valid() { return getDefaultProfile() != 0; }

    KisCompositeOpList userVisiblecompositeOps() const;
protected:

    void compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity);

private:

    Q_UINT8 * m_qcolordata;

    static const Q_UINT8 PIXEL_CYAN = 0;
    static const Q_UINT8 PIXEL_MAGENTA = 1;
    static const Q_UINT8 PIXEL_YELLOW = 2;
    static const Q_UINT8 PIXEL_BLACK = 3;
    static const Q_UINT8 PIXEL_CMYK_ALPHA = 4;
};

#endif // KIS_STRATEGY_COLORSPACE_CMYK_H_
