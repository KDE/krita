/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

    KoCompositeOpAlphaBase(const KoColorSpace * cs, const QString& id, const QString& description, const QString& category)
            : KoCompositeOp(cs, id, description, category) {
    }

public:
    using KoCompositeOp::composite;

    template<bool alphaLocked, bool allChannelFlags>
    void composite(quint8 *dstRowStart,
                   qint32 dststride,
                   const quint8 *srcRowStart,
                   qint32 srcstride,
                   const quint8 *maskRowStart,
                   qint32 maskstride,
                   qint32 rows,
                   qint32 cols,
                   quint8 U8_opacity,
                   const QBitArray & channelFlags) const {

        qint32 srcInc = (srcstride == 0) ? 0 : _CSTraits::channels_nb;

        channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);

        while (rows > 0) {
            const channels_type *srcN = reinterpret_cast<const channels_type *>(srcRowStart);
            channels_type *dstN = reinterpret_cast<channels_type *>(dstRowStart);
            const quint8 *mask = maskRowStart;

            qint32 columns = cols;

            while (columns > 0) {

                channels_type srcAlpha = _CSTraits::alpha_pos == -1 ? NATIVE_OPACITY_OPAQUE : _compositeOp::selectAlpha(srcN[_CSTraits::alpha_pos], dstN[_CSTraits::alpha_pos]);

                // apply the alphamask
                if (mask != 0) {
                    srcAlpha = KoColorSpaceMaths<channels_type, quint8>::multiply(srcAlpha, *mask, U8_opacity);
                    mask++;
                } else if (opacity != NATIVE_OPACITY_OPAQUE) {
                    srcAlpha = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, opacity);
                }

                if (srcAlpha != NATIVE_OPACITY_TRANSPARENT) {


                    channels_type dstAlpha = _CSTraits::alpha_pos == -1 ? NATIVE_OPACITY_OPAQUE : dstN[_CSTraits::alpha_pos];

                    channels_type srcBlend;

                    if (dstAlpha == NATIVE_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        channels_type newAlpha = dstAlpha + KoColorSpaceMaths<channels_type>::multiply(NATIVE_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        if (!alphaLocked && !_alphaLocked) { // No need to check for _CSTraits::alpha_pos == -1 since it is contained in alphaLocked
                            dstN[_CSTraits::alpha_pos] = newAlpha;
                        }

                        if (newAlpha != 0) {
                            srcBlend = KoColorSpaceMaths<channels_type>::divide(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }
                    _compositeOp::composeColorChannels(srcBlend, srcN, dstN, allChannelFlags, channelFlags);

                }
                columns--;
                srcN += srcInc;
                dstN += _CSTraits::channels_nb;
            }

            rows--;
            srcRowStart += srcstride;
            dstRowStart += dststride;
            if (maskRowStart) {
                maskRowStart += maskstride;
            }
        }
    }
    template<bool alphaLocked>
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
        bool allChannelFlags = channelFlags.isEmpty();
        if(allChannelFlags)
        {
            composite<alphaLocked, true>(dstRowStart, dststride, srcRowStart, srcstride, maskRowStart, maskstride, rows, cols, U8_opacity, channelFlags);
        } else {
            composite<alphaLocked, false>(dstRowStart, dststride, srcRowStart, srcstride, maskRowStart, maskstride, rows, cols, U8_opacity, channelFlags);
        }
    }
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
        bool alphaLocked = false;
        if (!channelFlags.isEmpty()) {
            if (_CSTraits::alpha_pos == -1 || !channelFlags.testBit(_CSTraits::alpha_pos)) {
                alphaLocked = true;
            }
        }
        if(alphaLocked)
        {
            composite<true>(dstRowStart, dststride, srcRowStart, srcstride, maskRowStart, maskstride, rows, cols, U8_opacity, channelFlags);
        } else {
            composite<false>(dstRowStart, dststride, srcRowStart, srcstride, maskRowStart, maskstride, rows, cols, U8_opacity, channelFlags);
        }
    }
};

#endif
