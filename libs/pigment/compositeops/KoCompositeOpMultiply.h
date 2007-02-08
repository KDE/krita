/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef KOCOMPOSITEOPMULTIPLY_H_
#define KOCOMPOSITEOPMULTIPLY_H_

#include "KoColorSpaceMaths.h"
#include "KoCompositeOp.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::max()
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::min()

/**
 * A template version of the over composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpMultiply : public KoCompositeOp {
    typedef typename _CSTraits::channels_type channels_type;
    public:

        KoCompositeOpMultiply(KoColorSpace * cs)
        : KoCompositeOp(cs, COMPOSITE_MULT, i18n("Multiply" ) )
        {
        }

    public:

        void composite(quint8 *dstRowStart,
                        qint32 dststride,
                        const quint8 *srcRowStart,
                        qint32 srcstride,
                        const quint8 *maskRowStart,
                        qint32 maskstride,
                        qint32 rows,
                        qint32 cols,
                        quint8 U8_opacity,
                        const QBitArray & channelFlags) const
        {
            Q_UNUSED( channelFlags );
            
            channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);
            qint32 pixelSize = colorSpace()->pixelSize();
            
            while (rows > 0) {
                const channels_type *src = reinterpret_cast<const channels_type *>(srcRowStart);
                channels_type *dst = reinterpret_cast<channels_type *>(dstRowStart);
                const quint8 *mask = maskRowStart;
                qint32 columns = cols;
        
                while (columns > 0) {
        
                    channels_type srcAlpha = src[_CSTraits::alpha_pos];
                    channels_type dstAlpha = dst[_CSTraits::alpha_pos];
        
                    srcAlpha = qMin(srcAlpha, dstAlpha);
        
                    // apply the alphamask
                    if(mask != 0)
                    {
                        if(*mask != OPACITY_OPAQUE)
                        {
                            srcAlpha = KoColorSpaceMaths<channels_type,quint8>::multiply(srcAlpha, *mask);
                        }
                        mask++;
                    }
        
        
                    if (srcAlpha != NATIVE_OPACITY_TRANSPARENT) {
        
                        if (opacity != NATIVE_OPACITY_OPAQUE) {
                            srcAlpha = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, opacity);
                        }
        
                        channels_type srcBlend;
        
                        if (dstAlpha == NATIVE_OPACITY_OPAQUE) {
                            srcBlend = srcAlpha;
                        } else {
                            channels_type newAlpha = dstAlpha + KoColorSpaceMaths<channels_type>::multiply(NATIVE_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                            dst[_CSTraits::alpha_pos] = newAlpha;
        
                            if (newAlpha != 0) {
                                srcBlend = KoColorSpaceMaths<channels_type>::divide(srcAlpha, newAlpha);
                            } else {
                                srcBlend = srcAlpha;
                            }
                        }
                        for(uint i = 0; i < _CSTraits::channels_nb; i++)
                        {
                          if( (int)i != _CSTraits::alpha_pos)
                          {
                            channels_type srcColor = src[i];
                            channels_type dstColor = dst[i];
            
                            srcColor = KoColorSpaceMaths<channels_type>::multiply(srcColor, dstColor);
            
                            dst[i] = KoColorSpaceMaths<channels_type>::blend(srcColor, dstColor, srcBlend);
                          
                          }
                        }
                    }
        
                    columns--;
                    src += _CSTraits::channels_nb;
                    dst += _CSTraits::channels_nb;
                }
        
                rows--;
                srcRowStart += srcstride;
                dstRowStart += dststride;
                if(maskRowStart)
                    maskRowStart += maskstride;
            }

        }

};

#endif
