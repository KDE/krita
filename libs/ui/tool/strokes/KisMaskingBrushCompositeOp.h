/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISMASKINGBRUSHCOMPOSITEOP_H
#define KISMASKINGBRUSHCOMPOSITEOP_H

#include <KoColorSpaceTraits.h>
#include <KoGrayColorSpaceTraits.h>
#include <KoColorSpaceMaths.h>

#include "KisMaskingBrushCompositeOpBase.h"

template <typename channels_type, channels_type compositeFunc(channels_type, channels_type)>
class KisMaskingBrushCompositeOp : public KisMaskingBrushCompositeOpBase
{
public:
    KisMaskingBrushCompositeOp(int dstPixelSize, int dstAlphaOffset)
        : m_dstPixelSize(dstPixelSize),
          m_dstAlphaOffset(dstAlphaOffset)
    {
    }

    void composite(const quint8 *srcRowStart, int srcRowStride,
                   quint8 *dstRowStart, int dstRowStride,
                   int columns, int rows) override {

        using MaskPixel = KoGrayU8Traits::Pixel;

        dstRowStart += m_dstAlphaOffset;

        for (int y = 0; y < rows; y++) {
            const quint8 *srcPtr = srcRowStart;
            quint8 *dstPtr = dstRowStart;

            for (int x = 0; x < columns; x++) {

                const MaskPixel *srcDataPtr = reinterpret_cast<const MaskPixel*>(srcPtr);

                const quint8 mask = KoColorSpaceMaths<quint8>::multiply(srcDataPtr->gray, srcDataPtr->alpha);
                const channels_type maskScaled = KoColorSpaceMaths<quint8, channels_type>::scaleToA(mask);

                channels_type *dstDataPtr = reinterpret_cast<channels_type*>(dstPtr);
                *dstDataPtr = compositeFunc(maskScaled, *dstDataPtr);

                srcPtr += sizeof(MaskPixel);
                dstPtr += m_dstPixelSize;
            }

            srcRowStart += srcRowStride;
            dstRowStart += dstRowStride;
        }
    }

private:
    int m_dstPixelSize;
    int m_dstAlphaOffset;
};

#endif // KISMASKINGBRUSHCOMPOSITEOP_H
