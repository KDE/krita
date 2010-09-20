/*
 *  Copyright (c) 2006, 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_COMPOSITE_COPY_OP2_H
#define KO_COMPOSITE_COPY_OP2_H

#include <KoColorSpaceMaths.h>

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

/**
 * Generic implementation of the COPY composite op. That respect selection.
 */
template<class _CSTraits>
class KoCompositeOpCopy2 : public KoCompositeOp
{

    typedef typename _CSTraits::channels_type channels_type;

public:

    explicit KoCompositeOpCopy2(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Copy"), KoCompositeOp::categoryMix(), false) {
    }

public:
    using KoCompositeOp::composite;

    void composite(quint8 *dstRowStart,
                   qint32 dstRowStride,
                   const quint8 *srcRowStart,
                   qint32 srcRowStride,
                   const quint8 *maskRowStart,
                   qint32 maskRowStride,
                   qint32 rows,
                   qint32 cols,
                   quint8 U8_opacity,
                   const QBitArray & channelFlags) const
    {
        Q_UNUSED(channelFlags);
        if(maskRowStart == 0 && U8_opacity == OPACITY_OPAQUE_U8)
        {
            quint8 *dst = dstRowStart;
            const quint8 *src = srcRowStart;
            quint8 bytesPerPixel = _CSTraits::pixelSize;
            while (rows > 0) {
                if (srcRowStride == 0) {
                    quint8* dstN = dst;
                    qint32 columns = cols;
                    while (columns > 0) {
                        memcpy(dstN, src, bytesPerPixel);
                        dstN += bytesPerPixel;
                        --columns;
                    }
                } else {
                    memcpy(dst, src, cols * bytesPerPixel);
                }

                dst += dstRowStride;
                src += srcRowStride;
                --rows;
            }
        } else {
            qint32 srcInc = (srcRowStride == 0) ? 0 : _CSTraits::channels_nb;
            channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);
            while (rows > 0)
            {
                const channels_type *srcN = reinterpret_cast<const channels_type *>(srcRowStart);
                channels_type *dstN = reinterpret_cast<channels_type *>(dstRowStart);
                const quint8 *mask = maskRowStart;

                qint32 columns = cols;

                while (columns > 0)
                {
                    channels_type blend;
                    // compute the blend
                    if (mask == 0 || (*mask == OPACITY_OPAQUE_U8)) {
                        blend = opacity;
                    } else {
                        blend = KoColorSpaceMaths<channels_type, quint8>::multiply(opacity, *mask);
                        ++mask;
                    }
                    
                    for (uint i = 0; i < _CSTraits::channels_nb; i++) {
                        dstN[i] = KoColorSpaceMaths<channels_type>::blend(srcN[i], dstN[i], blend);
                    }
                    --columns;
                    srcN += srcInc;
                    dstN += _CSTraits::channels_nb;
                }

                rows--;
                srcRowStart += srcRowStride;
                dstRowStart += dstRowStride;
                if (maskRowStart) {
                    maskRowStart += maskRowStride;
                }
            }
            
        }
    }
};

#endif
