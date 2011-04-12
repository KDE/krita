/*
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

#ifndef KOCOMPOSITEO_BASE_H_
#define KOCOMPOSITEO_BASE_H_

#include <KoCompositeOp.h>
#include "KoCompositeOpFunctions.h"

/**
 * A template base class that can be used for most composite modes/ops
 *
 * @param _compositeOp this template parameter is a class that must be
 *        derived fom KoCompositeOpBase and must define the static member function
 *        template<bool alphaLocked, bool allChannelFlags>
 *        inline static channels_type composeColorChannels(
 *            const channels_type* src,
 *            channels_type srcAlpha,
 *            channels_type* dst,
 *            channels_type dstAlpha,
 *            channels_type opacity,
 *            const QBitArray& channelFlags
 *        )
 *
 *        where channels_type is _CSTraits::channels_type
 */
template<class _CSTraits, class _compositeOp>
class KoCompositeOpBase : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    static const qint32 channels_nb = _CSTraits::channels_nb;
    static const qint32 alpha_pos   = _CSTraits::alpha_pos;
    
public:
    KoCompositeOpBase(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category, bool userVisible)
        : KoCompositeOp(cs, id, description, category, userVisible) { }

    using KoCompositeOp::composite;
    
    virtual void composite(quint8*       dstRowStart , qint32 dstRowStride ,
                           const quint8* srcRowStart , qint32 srcRowStride ,
                           const quint8* maskRowStart, qint32 maskRowStride,
                           qint32 rows, qint32 cols, quint8 U8_opacity, const QBitArray& channelFlags) const {
        
        const QBitArray& flags           = channelFlags.isEmpty() ? QBitArray(channels_nb,true) : channelFlags;
        bool             allChannelFlags = channelFlags.isEmpty() || channelFlags == QBitArray(channels_nb,true);
        bool             alphaLocked     = (alpha_pos != -1) && !flags.testBit(alpha_pos);
        
        if(alphaLocked) {
            if(allChannelFlags)
                genericComposite<true,true>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
            else
                genericComposite<true,false>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
        }
        else {
            if(allChannelFlags)
                genericComposite<false,true>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
            else
                genericComposite<false,false>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
        }
    }

private:
    template<bool alphaLocked, bool allChannelFlags>
    void genericComposite(quint8*       dstRowStart , qint32 dstRowStride ,
                          const quint8* srcRowStart , qint32 srcRowStride ,
                          const quint8* maskRowStart, qint32 maskRowStride,
                          qint32 rows, qint32 cols, quint8 U8_opacity, const QBitArray& channelFlags) const {
        
        using namespace Arithmetic;
        
        qint32        srcInc    = (srcRowStride == 0) ? 0 : channels_nb;
        bool          useMask   = maskRowStart != 0;
        channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
        channels_type opacity   = KoColorSpaceMaths<quint8,channels_type>::scaleToA(U8_opacity);
        
        for(; rows>0; --rows) {
            const channels_type* src  = reinterpret_cast<const channels_type*>(srcRowStart);
            channels_type*       dst  = reinterpret_cast<channels_type*>(dstRowStart);
            const quint8*        mask = maskRowStart;
            
            for(qint32 c=cols; c>0; --c) {
                channels_type srcAlpha = (alpha_pos == -1) ? unitValue : src[alpha_pos];
                channels_type dstAlpha = (alpha_pos == -1) ? unitValue : dst[alpha_pos];
                channels_type blend    = useMask ? mul(opacity, scale<channels_type>(*mask)) : opacity;
                
                channels_type newDstAlpha = _compositeOp::template composeColorChannels<alphaLocked,allChannelFlags>(
                    src, srcAlpha, dst, dstAlpha, blend, channelFlags
                );
                
                if(alpha_pos != -1)
                    dst[alpha_pos] = alphaLocked ? dstAlpha : newDstAlpha;
                
                src += srcInc;
                dst += channels_nb;
                ++mask;
            }
            
            srcRowStart  += srcRowStride;
            dstRowStart  += dstRowStride;
            maskRowStart += maskRowStride;
        }
    }
};

#endif // KOCOMPOSITEO_BASE_H_
