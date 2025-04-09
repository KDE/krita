/*
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2025 Carsten Hartenfels <carsten.hartenfels@pm.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCOMPOSITEOPMARKER_H_
#define KOCOMPOSITEOPMARKER_H_

#include "KoAlphaDarkenParamsWrapper.h"
#include "KoCompositeOpAlphaDarken.h"

template<class Traits>
class KoCompositeOpMarker : public KoCompositeOp
{
    typedef typename Traits::channels_type channels_type;

    static constexpr qint32 channels_nb = Traits::channels_nb;
    static constexpr qint32 alpha_pos = Traits::alpha_pos;
    static constexpr qint32 pixel_size = Traits::pixelSize;

public:
    KoCompositeOpMarker(const KoColorSpace *cs)
        : KoCompositeOp(cs, COMPOSITE_MARKER, KoCompositeOp::categoryMix())
    {
    }

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo &params) const override
    {
        bool useMask = params.maskRowStart != 0;
        const QBitArray &flags = params.channelFlags.isEmpty() ? QBitArray(channels_nb, true) : params.channelFlags;
        bool allChannelFlags = params.channelFlags.isEmpty() || params.channelFlags == QBitArray(channels_nb, true);
        bool alphaLocked = (alpha_pos != -1) && !flags.testBit(alpha_pos);
        if (useMask) {
            if (alphaLocked) {
                if (allChannelFlags) {
                    genericComposite<true, true, true>(params, &flags);
                } else {
                    genericComposite<true, true, false>(params, &flags);
                }
            } else {
                if (allChannelFlags) {
                    genericComposite<true, false, true>(params, &flags);
                } else {
                    genericComposite<true, false, false>(params, &flags);
                }
            }
        } else {
            if (alphaLocked) {
                if (allChannelFlags) {
                    genericComposite<false, true, true>(params, &flags);
                } else {
                    genericComposite<false, true, false>(params, &flags);
                }
            } else {
                if (allChannelFlags) {
                    genericComposite<false, false, true>(params, &flags);
                } else {
                    genericComposite<false, false, false>(params, &flags);
                }
            }
        }
    }

    template<bool useMask, bool alphaLocked, bool allChannelFlags>
    void genericComposite(const KoCompositeOp::ParameterInfo &params, const QBitArray *channelFlags) const
    {
        using namespace Arithmetic;

        KoAlphaDarkenParamsWrapperCreamy paramsWrapper(params);

        qint32 srcInc = (params.srcRowStride == 0) ? 0 : channels_nb;
        channels_type flow = scale<channels_type>(paramsWrapper.flow);
        channels_type opacity = scale<channels_type>(paramsWrapper.opacity);
        quint8 *dstRowStart = params.dstRowStart;
        const quint8 *srcRowStart = params.srcRowStart;
        const quint8 *maskRowStart = params.maskRowStart;

        for (qint32 r = params.rows; r > 0; --r) {
            const channels_type *src = reinterpret_cast<const channels_type *>(srcRowStart);
            channels_type *dst = reinterpret_cast<channels_type *>(dstRowStart);
            const quint8 *mask = maskRowStart;

            for (qint32 c = params.cols; c > 0; --c) {
                channels_type srcAlpha = (alpha_pos == -1) ? unitValue<channels_type>() : src[alpha_pos];
                channels_type dstAlpha = (alpha_pos == -1) ? unitValue<channels_type>() : dst[alpha_pos];
                channels_type mskAlpha = useMask ? mul(scale<channels_type>(*mask), srcAlpha) : srcAlpha;

                srcAlpha = mul(mskAlpha, opacity);

                if (dstAlpha != zeroValue<channels_type>()) {
                    channels_type t = KoColorSpaceMaths<channels_type>::clampAfterScale(
                        KoColorSpaceMaths<channels_type>::divide(srcAlpha, dstAlpha));
                    for (qint32 i = 0; i < channels_nb; i++) {
                        if (i != alpha_pos && (allChannelFlags || channelFlags->testBit(i))) {
                            dst[i] = lerp(dst[i], src[i], t);
                        }
                    }
                } else {
                    if constexpr (!allChannelFlags) {
                        memset(reinterpret_cast<quint8 *>(dst), 0, pixel_size);
                    }
                    for (qint32 i = 0; i < channels_nb; i++) {
                        if (i != alpha_pos && (allChannelFlags || channelFlags->testBit(i))) {
                            dst[i] = src[i];
                        }
                    }
                }

                if constexpr (alpha_pos != -1 && !alphaLocked) {
                    dst[alpha_pos] = KoCompositeOpAlphaDarken<Traits, KoAlphaDarkenParamsWrapperCreamy>::calculateAlpha(
                        params,
                        paramsWrapper,
                        flow,
                        opacity,
                        srcAlpha,
                        dstAlpha,
                        mskAlpha);
                }

                src += srcInc;
                dst += channels_nb;

                if constexpr (useMask) {
                    ++mask;
                }
            }

            srcRowStart += params.srcRowStride;
            dstRowStart += params.dstRowStride;
            maskRowStart += params.maskRowStride;
        }
    }
};

#endif // KOCOMPOSITEOPMARKER_H_
