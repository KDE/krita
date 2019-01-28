/*
 * Copyright (c) 2006 Cyrille Berger  <cberger@cberger.net>
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCOMPOSITEOPALPHADARKEN_H_
#define KOCOMPOSITEOPALPHADARKEN_H_

#include "KoCompositeOpFunctions.h"
#include "KoCompositeOpBase.h"
#include <KoCompositeOpRegistry.h>

/**
 * A template version of the alphadarken composite operation to use in colorspaces
 */
template<class Traits>
class KoCompositeOpAlphaDarken: public KoCompositeOp
{
    typedef typename Traits::channels_type channels_type;

    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;

public:
    KoCompositeOpAlphaDarken(const KoColorSpace* cs):
        KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, i18n("Alpha darken"), KoCompositeOp::categoryMix()) { }

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        if(params.maskRowStart != 0)
            genericComposite<true>(params);
        else
            genericComposite<false>(params);
    }

    template<bool useMask>
    void genericComposite(const KoCompositeOp::ParameterInfo& params) const
    {
        using namespace Arithmetic;

        qint32        srcInc       = (params.srcRowStride == 0) ? 0 : channels_nb;
        channels_type flow         = scale<channels_type>(params.flow);
        channels_type opacity      = mul(flow, scale<channels_type>(params.opacity));
        quint8*       dstRowStart  = params.dstRowStart;
        const quint8* srcRowStart  = params.srcRowStart;
        const quint8* maskRowStart = params.maskRowStart;

        for(quint32 r=params.rows; r>0; --r) {
            const channels_type* src  = reinterpret_cast<const channels_type*>(srcRowStart);
            channels_type*       dst  = reinterpret_cast<channels_type*>(dstRowStart);
            const quint8*        mask = maskRowStart;

            for(qint32 c=params.cols; c>0; --c) {
                channels_type srcAlpha = (alpha_pos == -1) ? unitValue<channels_type>() : src[alpha_pos];
                channels_type dstAlpha = (alpha_pos == -1) ? unitValue<channels_type>() : dst[alpha_pos];
                channels_type mskAlpha = useMask ? mul(scale<channels_type>(*mask), srcAlpha) : srcAlpha;

                srcAlpha = mul(mskAlpha, opacity);

                if(dstAlpha != zeroValue<channels_type>()) {
                    for(qint32 i=0; i <channels_nb; i++) {
                        if(i != alpha_pos)
                            dst[i] = lerp(dst[i], src[i], srcAlpha);
                    }
                }
                else {
                    for(qint32 i=0; i <channels_nb; i++) {
                        if(i != alpha_pos)
                            dst[i] = src[i];
                    }
                }

                if(alpha_pos != -1) {
                    channels_type fullFlowAlpha;
                    channels_type averageOpacity = mul(flow, scale<channels_type>(*params.lastOpacity));

                    if (averageOpacity > opacity) {
                        channels_type reverseBlend = KoColorSpaceMaths<channels_type>::divide(dstAlpha, averageOpacity);
                        fullFlowAlpha = averageOpacity > dstAlpha ? lerp(srcAlpha, averageOpacity, reverseBlend) : dstAlpha;
                    } else {
                        fullFlowAlpha = opacity > dstAlpha ? lerp(dstAlpha, opacity, mskAlpha) : dstAlpha;
                    }

                    if (params.flow == 1.0) {
                        dstAlpha = fullFlowAlpha;
                    } else {
                        channels_type zeroFlowAlpha = unionShapeOpacity(srcAlpha, dstAlpha);
                        dstAlpha = lerp(zeroFlowAlpha, fullFlowAlpha, flow);
                    }

                    dst[alpha_pos] = dstAlpha;
                }

                src += srcInc;
                dst += channels_nb;

                if(useMask)
                    ++mask;
            }

            srcRowStart  += params.srcRowStride;
            dstRowStart  += params.dstRowStride;
            maskRowStart += params.maskRowStride;
        }
    }
};

#endif // KOCOMPOSITEOPALPHADARKEN_H_
