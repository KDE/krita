/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOCOMPOSITEOP_BASE_H_
#define KOCOMPOSITEOP_BASE_H_

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
 *            channels_type maskAlpha,
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
    static const qint32 pixel_size   = _CSTraits::pixelSize;

public:
    KoCompositeOpBase(const KoColorSpace* cs, const QString& id, const QString& category)
        : KoCompositeOp(cs, id, category) { }

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override {

        const QBitArray& flags           = params.channelFlags.isEmpty() ? QBitArray(channels_nb,true) : params.channelFlags;
        bool             allChannelFlags = params.channelFlags.isEmpty() || params.channelFlags == QBitArray(channels_nb,true);
        bool             alphaLocked     = (alpha_pos != -1) && !flags.testBit(alpha_pos);
        bool             useMask         = params.maskRowStart != 0;

        if(useMask) {
            if(alphaLocked) {
                if(allChannelFlags) { genericComposite<true,true,true> (params, flags); }
                else                { genericComposite<true,true,false>(params, flags); }
            }
            else {
                if(allChannelFlags) { genericComposite<true,false,true> (params, flags); }
                else                { genericComposite<true,false,false>(params, flags); }
            }
        }
        else {
            if(alphaLocked) {
                if(allChannelFlags) { genericComposite<false,true,true> (params, flags); }
                else                { genericComposite<false,true,false>(params, flags); }
            }
            else {
                if(allChannelFlags) { genericComposite<false,false,true> (params, flags); }
                else                { genericComposite<false,false,false>(params, flags); }
            }
        }
    }

private:
    template<bool useMask, bool alphaLocked, bool allChannelFlags>
    void genericComposite(const KoCompositeOp::ParameterInfo& params, const QBitArray& channelFlags) const {

        using namespace Arithmetic;

        qint32        srcInc       = (params.srcRowStride == 0) ? 0 : channels_nb;
        channels_type opacity      = scale<channels_type>(params.opacity);
        quint8*       dstRowStart  = params.dstRowStart;
        const quint8* srcRowStart  = params.srcRowStart;
        const quint8* maskRowStart = params.maskRowStart;

        for (qint32 r=0; r<params.rows; ++r) {
            const channels_type* src  = reinterpret_cast<const channels_type*>(srcRowStart);
            channels_type*       dst  = reinterpret_cast<channels_type*>(dstRowStart);
            const quint8*        mask = maskRowStart;

            for (qint32 c=0; c<params.cols; ++c) {
                channels_type srcAlpha = (alpha_pos == -1) ? unitValue<channels_type>() : src[alpha_pos];
                channels_type dstAlpha = (alpha_pos == -1) ? unitValue<channels_type>() : dst[alpha_pos];
                channels_type mskAlpha = useMask ? scale<channels_type>(*mask) : unitValue<channels_type>();

                if (!allChannelFlags && dstAlpha == zeroValue<channels_type>()) {
                    memset(reinterpret_cast<quint8*>(dst), 0, pixel_size);
                }

                channels_type newDstAlpha = _compositeOp::template composeColorChannels<alphaLocked,allChannelFlags>(
                    src, srcAlpha, dst, dstAlpha, mskAlpha, opacity, channelFlags
                );

                if(alpha_pos != -1)
                    dst[alpha_pos] = alphaLocked ? dstAlpha : newDstAlpha;

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

#endif // KOCOMPOSITEOP_BASE_H_
