/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KOCOMPOSITEOPALPHABASE_H_
#define _KOCOMPOSITEOPALPHABASE_H_

#include <QBitArray>

#include <KoColorSpaceMaths.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceConstants.h>

#define NATIVE_MIN_VALUE KoColorSpaceMathsTraits<channels_type>::min
#define NATIVE_MAX_VALUE KoColorSpaceMathsTraits<channels_type>::max
#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue
#define NATIVE_ZERO_VALUE KoColorSpaceMathsTraits<channels_type>::zeroValue


/**
 * A template base class for all composite op that compose color
 * channels values for colorspaces that have an alpha channel.
 *
 * @param _compositeOp this class should define a function with the
 *        following signature: inline static void composeColorChannels
 */
template<class _CSTraits, class _compositeOp, bool _alphaLocked>
class KoCompositeOpAlphaBase : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
public:

    KoCompositeOpAlphaBase(const KoColorSpace * cs, const QString& id, const QString& category)
            : KoCompositeOp(cs, id, category) {
    }

public:
    using KoCompositeOp::composite;

    template<bool alphaLocked, bool allChannelFlags>
    void composite(const KoCompositeOp::ParameterInfo& params) const {
        qint32 srcInc = (params.srcRowStride == 0) ? 0 : _CSTraits::channels_nb;

        channels_type opacity = KoColorSpaceMaths<float, channels_type>::scaleToA(params.opacity);

        const quint8 *srcRowStart = params.srcRowStart;
        quint8 *dstRowStart = params.dstRowStart;
        const quint8 *maskRowStart = params.maskRowStart;

        qint32 rows = params.rows;

        while (rows > 0) {
            const channels_type *srcN = reinterpret_cast<const channels_type *>(srcRowStart);
            channels_type *dstN = reinterpret_cast<channels_type *>(dstRowStart);
            const quint8 *mask = maskRowStart;

            qint32 columns = params.cols;

            while (columns > 0) {

                channels_type srcAlpha = _CSTraits::alpha_pos == -1 ? NATIVE_OPACITY_OPAQUE : _compositeOp::selectAlpha(srcN[_CSTraits::alpha_pos], dstN[_CSTraits::alpha_pos]);

                // apply the alphamask
                if (mask != 0) {
                    srcAlpha = KoColorSpaceMaths<quint8, channels_type>::multiply(*mask, srcAlpha, opacity);
                    mask++;
                } else if (opacity != NATIVE_OPACITY_OPAQUE) {
                    srcAlpha = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, opacity);
                }

                if (srcAlpha != NATIVE_OPACITY_TRANSPARENT) {


                    channels_type dstAlpha = _CSTraits::alpha_pos == -1 ? NATIVE_OPACITY_OPAQUE : dstN[_CSTraits::alpha_pos];

                    channels_type srcBlend;

                    if (alphaLocked || _alphaLocked ||
                        dstAlpha == NATIVE_OPACITY_OPAQUE) {

                        srcBlend = srcAlpha;

                    } else if (dstAlpha == NATIVE_OPACITY_TRANSPARENT) {
                        if (!allChannelFlags) {
                            for (int i = 0; i < (int)_CSTraits::channels_nb; i++) {
                                if (i != _CSTraits::alpha_pos) {
                                    dstN[i] = NATIVE_ZERO_VALUE;
                                }
                            }
                        }

                        if (!alphaLocked && !_alphaLocked) {
                            dstN[_CSTraits::alpha_pos] = srcAlpha;
                        }
                        srcBlend = NATIVE_OPACITY_OPAQUE;

                    } else {
                        channels_type newAlpha = dstAlpha + KoColorSpaceMaths<channels_type>::multiply(NATIVE_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        if (!alphaLocked && !_alphaLocked) { // No need to check for _CSTraits::alpha_pos == -1 since it is contained in alphaLocked
                            dstN[_CSTraits::alpha_pos] = newAlpha;
                        }
                        // newAlpha cannot be zero, because srcAlpha!=zero and dstAlpha!=unit here
                        srcBlend = KoColorSpaceMaths<channels_type>::divide(srcAlpha, newAlpha);
                    }
                    _compositeOp::composeColorChannels(srcBlend, srcN, dstN, allChannelFlags, params.channelFlags);

                }
                columns--;
                srcN += srcInc;
                dstN += _CSTraits::channels_nb;
            }

            rows--;
            srcRowStart += params.srcRowStride;
            dstRowStart += params.dstRowStride;
            if (maskRowStart) {
                maskRowStart += params.maskRowStride;
            }
        }
    }
    template<bool alphaLocked>
    void composite(const KoCompositeOp::ParameterInfo& params) const
    {
        bool allChannelFlags = params.channelFlags.isEmpty();
        if(allChannelFlags)
        {
            composite<alphaLocked, true>(params);
        } else {
            composite<alphaLocked, false>(params);
        }
    }

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        bool alphaLocked = false;
        if (!params.channelFlags.isEmpty()) {
            if (_CSTraits::alpha_pos == -1 || !params.channelFlags.testBit(_CSTraits::alpha_pos)) {
                alphaLocked = true;
            }
        }
        if(alphaLocked)
        {
            composite<true>(params);
        } else {
            composite<false>(params);
        }
    }
};

#endif
