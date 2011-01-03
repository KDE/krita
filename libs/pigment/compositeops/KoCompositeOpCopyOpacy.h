/*
 *  Copyright (c) 2008 Silvio Heinrich <plassy@web.de>
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

#ifndef KOCOMPOSITEOPCOPYOPACY_H_
#define KOCOMPOSITEOPCOPYOPACY_H_

#include <KoColorSpaceMaths.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceConstants.h>

/**
 * A template version of the copy opacy composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpCopyOpacy : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    static const qint32 channels_nb = _CSTraits::channels_nb;
    static const qint32 alpha_pos   = _CSTraits::alpha_pos;
    static const qint32 pixelSize   = _CSTraits::pixelSize;
    
public:
    KoCompositeOpCopyOpacy(const KoColorSpace * cs)
        : KoCompositeOp(cs, COMPOSITE_COPY_OPACITY, i18n("Copy Opacy"), KoCompositeOp::categoryArithmetic(), true) {
    }
    
public:
    inline static channels_type selectAlpha(channels_type srcAlpha, channels_type dstAlpha) {
        return qMin(srcAlpha, dstAlpha);
    }
    
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
        bool          useMask = maskRowStart != 0;
        channels_type opacity = KoColorSpaceMaths<quint8,channels_type>::scaleToA(U8_opacity);
        
        if(srcRowStride != 0) {
            for(; rows>0; --rows) {
                const quint8*        mskRowItr = maskRowStart;
                const channels_type* srcRowItr = reinterpret_cast<const channels_type*>(srcRowStart) + alpha_pos;
                channels_type*       dstRowItr = reinterpret_cast<channels_type*>(dstRowStart) + alpha_pos;
                
                for(qint32 c=cols; c>0; --c) {
                    channels_type value = 0;
                    
                    switch(U8_opacity)
                    {
                    case OPACITY_TRANSPARENT_U8: { value = *dstRowItr; } break;
                    case OPACITY_OPAQUE_U8:      { value = *srcRowItr; } break;
                    default: { value = KoColorSpaceMaths<channels_type>::blend(*srcRowItr, *dstRowItr, opacity); } break;
                    }
                    
                    if(useMask) {
                        channels_type blend = KoColorSpaceMaths<quint8,channels_type>::scaleToA(*mskRowItr);
                        value = KoColorSpaceMaths<channels_type>::blend(value, *dstRowItr, blend);
                        ++mskRowItr;
                    }
                    
                    value      = (value > *dstRowItr) ? (value-1) : value;
                    *dstRowItr = value;
                    srcRowItr += channels_nb;
                    dstRowItr += channels_nb;
                }
                
                srcRowStart  += srcRowStride;
                dstRowStart  += dstRowStride;
                maskRowStart += maskRowStride;
            }
        }
        else {
            channels_type srcValue = *(reinterpret_cast<const channels_type*>(srcRowStart) + alpha_pos);
            
            for(; rows>0; --rows) {
                const quint8*  mskRowItr = maskRowStart;
                channels_type* dstRowItr = reinterpret_cast<channels_type*>(dstRowStart) + alpha_pos;
                
                for(qint32 c=cols; c>0; --c) {
                    channels_type value = 0;
                    
                    switch(U8_opacity)
                    {
                        case OPACITY_TRANSPARENT_U8: { value = *dstRowItr; } break;
                        case OPACITY_OPAQUE_U8:      { value = srcValue;   } break;
                        default: { value = KoColorSpaceMaths<channels_type>::blend(srcValue, *dstRowItr, opacity); } break;
                    }
                    
                    if(useMask) {
                        channels_type blend = KoColorSpaceMaths<quint8,channels_type>::scaleToA(*mskRowItr);
                        value = KoColorSpaceMaths<channels_type>::blend(value, *dstRowItr, blend);
                        ++mskRowItr;
                    }
                    
                    value      = (value > *dstRowItr) ? (value-1) : value;
                    *dstRowItr = value;
                    dstRowItr += channels_nb;
                }
                
                srcRowStart  += srcRowStride;
                dstRowStart  += dstRowStride;
                maskRowStart += maskRowStride;
            }
        }
    }
    
};

#endif // KOCOMPOSITEOPCOPYOPACY_H_
