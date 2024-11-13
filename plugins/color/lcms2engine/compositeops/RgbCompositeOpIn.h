/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef RGBCOMPOSITEOPIN_H
#define RGBCOMPOSITEOPIN_H

#include <KoCompositeOp.h>

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< channels_type, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v ) KoColorSpaceMaths< float, channels_type>::scaleToA( v )

template<class _CSTraits>
class RgbCompositeOpIn : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpIn(KoColorSpace *cs)
        : KoCompositeOp(cs, COMPOSITE_IN, "")
    {
    }

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override
    {
        channels_type opacity = KoColorSpaceMaths<float, channels_type>::scaleToA(params.opacity);

        if (opacity == NATIVE_OPACITY_TRANSPARENT) {
            return;
        }

        const quint8 *srcRowStart = params.srcRowStart;
        quint8 *dstRowStart = params.dstRowStart;

        qint32 rows = params.rows;

        qint32 i;

        //qreal sAlpha, dAlpha;
        qreal alpha;

        while (rows-- > 0) {
            channels_type *d = reinterpret_cast<channels_type *>(dstRowStart);
            const channels_type *s = reinterpret_cast<const channels_type *>(srcRowStart);

            for (i = params.cols; i > 0; i--, d += _CSTraits::channels_nb, s += _CSTraits::channels_nb) {

                if (s[_CSTraits::alpha_pos] == NATIVE_OPACITY_TRANSPARENT) {
                    d[_CSTraits::alpha_pos] = NATIVE_OPACITY_TRANSPARENT;
                    continue;
                } else if (s[_CSTraits::alpha_pos] == NATIVE_OPACITY_OPAQUE) {
                    continue;
                }
                if (d[_CSTraits::alpha_pos] == NATIVE_OPACITY_TRANSPARENT) {
                    continue;
                }

                alpha = (qreal)(s[_CSTraits::alpha_pos]) * d[_CSTraits::alpha_pos] / NATIVE_OPACITY_OPAQUE;

                if (params.channelFlags.isEmpty() || params.channelFlags.testBit(_CSTraits::alpha_pos)) {
                    d[_CSTraits::alpha_pos] = (channels_type)((d[_CSTraits::alpha_pos] * alpha / NATIVE_OPACITY_OPAQUE) + 0.5);
                }

            }
            dstRowStart += params.dstRowStride;
            srcRowStart += params.srcRowStride;
        }
    }
};

#endif
