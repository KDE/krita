/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOCOMPOSITEOPERASE_H_
#define KOCOMPOSITEOPERASE_H_

#include "KoColorSpaceMaths.h"
#include "KoCompositeOp.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

/**
 * A template version of the erase composite operation to use in colorspaces<
 */
template<class _CSTraits>
class KoCompositeOpErase : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;

public:

    KoCompositeOpErase(const KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_ERASE, i18n("Erase"), KoCompositeOp::categoryMix()) {
    }

public:
    using KoCompositeOp::composite;

    void composite(quint8 *dstRowStart,
                   qint32 dststride,
                   const quint8 *srcRowStart,
                   qint32 srcstride,
                   const quint8 *maskRowStart,
                   qint32 maskstride,
                   qint32 rows,
                   qint32 cols,
                   quint8 U8_opacity,
                   const QBitArray & channelFlags) const override {
        // XXX: How to use channelflags here? It would be cool to
        // erase all green from an image, for example.
        qint32 srcInc = (srcstride == 0) ? 0 : _CSTraits::channels_nb;
        channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);
        Q_UNUSED(channelFlags);
        while (rows-- > 0) {
            const channels_type *s = reinterpret_cast<const channels_type *>(srcRowStart);
            channels_type *d = reinterpret_cast<channels_type *>(dstRowStart);
            const quint8 *mask = maskRowStart;

            for (qint32 i = cols; i > 0; i--, s += srcInc, d += _CSTraits::channels_nb) {
                channels_type srcAlpha = s[_CSTraits::alpha_pos];

                // apply the alphamask
                if (mask != 0) {
                    quint8 U8_mask = *mask;

                    if (U8_mask != OPACITY_TRANSPARENT_U8) {
                        srcAlpha = KoColorSpaceMaths<channels_type>::
                            multiply(srcAlpha, KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_mask));
                    } else {
                        srcAlpha = 0;
                    }
                    ++mask;
                }
                srcAlpha = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, opacity);
                srcAlpha = NATIVE_OPACITY_OPAQUE - srcAlpha;
                d[_CSTraits::alpha_pos] = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, d[_CSTraits::alpha_pos]);
            }

            dstRowStart += dststride;
            srcRowStart += srcstride;
            if (maskRowStart) {
                maskRowStart += maskstride;
            }
        }
    }
};

#endif
