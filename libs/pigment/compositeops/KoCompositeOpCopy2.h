/*
 *  Copyright (c) 2006, 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

/**
 * Generic implementation of the COPY composite op. That respect selection.
 */
template<class _CSTraits>
class KoCompositeOpCopy2 : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    static const qint32 channels_nb = _CSTraits::channels_nb;
    static const qint32 alpha_pos   = _CSTraits::alpha_pos;
    
    template<class T>
    inline static T mul(T a, T b) { return T(KoColorSpaceMaths<T>::multiply(a, b)); }
    
    template<class T>
    inline static T mul(T a, T b, T c) { return T(KoColorSpaceMaths<T>::multiply(a, b, c)); }
    
    template<class T>
    inline static T lerp(T a, T b, T alpha) { return KoColorSpaceMaths<T>::blend(b, a, alpha); }
    
    template<class TRet, class T>
    inline static TRet scale(T a) { return KoColorSpaceMaths<T,TRet>::scaleToA(a); }

public:
    explicit KoCompositeOpCopy2(KoColorSpace * cs)
        : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Copy"), KoCompositeOp::categoryMix(), false) { }

    using KoCompositeOp::composite;
    
    virtual void composite(quint8 *dstRowStart       , qint32 dstRowStride ,
                           const quint8 *srcRowStart , qint32 srcRowStride ,
                           const quint8 *maskRowStart, qint32 maskRowStride,
                           qint32 rows, qint32 cols, quint8 U8_opacity, const QBitArray& channelFlags) const {
        
        const QBitArray& flags   = channelFlags.isEmpty() ? QBitArray(channels_nb, true) : channelFlags;
        bool             useMask = maskRowStart != 0;
        qint32           srcInc  = srcRowStride != 0 ? channels_nb : 0;
        channels_type    opacity = KoColorSpaceMaths<quint8,channels_type>::scaleToA(U8_opacity);
        
        for(; rows>0; --rows) {
            const quint8*        msk = maskRowStart;
            const channels_type* src = reinterpret_cast<const channels_type*>(srcRowStart);
            channels_type*       dst = reinterpret_cast<channels_type*>(dstRowStart);
            
            for(qint32 c=cols; c>0; --c) {
                channels_type srcAlpha   = (alpha_pos != -1) ? scale<channels_type>(src[alpha_pos]) : KoColorSpaceMathsTraits<channels_type>::unitValue;
                channels_type dstAlpha   = (alpha_pos != -1) ? scale<channels_type>(dst[alpha_pos]) : KoColorSpaceMathsTraits<channels_type>::unitValue;
                channels_type blendAlpha = useMask ? mul(opacity, scale<channels_type>(*msk)) : opacity;
                channels_type blendColor = mul(srcAlpha, blendAlpha);
                
                if(dstAlpha != KoColorSpaceMathsTraits<channels_type>::zeroValue) {
                    // blend the color channels
                    for(qint32 i=0; i<channels_nb; ++i)
                        if(i != alpha_pos && flags.testBit(i)) 
                            dst[i] = lerp(dst[i], src[i], blendColor);
                }
                else {
                    // don't blend if the color of the destination is undefined (has zero opacity)
                    // copy the source channel instead
                    for(qint32 i=0; i<channels_nb; ++i)
                        if(i != alpha_pos && flags.testBit(i)) 
                            dst[i] = src[i];
                }
                
                // blend the alpha channel if there exists one
                if(alpha_pos != -1)
                    dst[alpha_pos] = lerp(dstAlpha, srcAlpha, blendAlpha);
                
                src += srcInc;
                dst += channels_nb;
                ++msk;
            }
            
            srcRowStart  += srcRowStride;
            dstRowStart  += dstRowStride;
            maskRowStart += maskRowStride;
        }
    }
};

#endif
