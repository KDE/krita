/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KOCOMPOSITEOPNONALPHABASE_H_
#define _KOCOMPOSITEOPNONALPHABASE_H_

#include <QBitArray>

#include "KoColorSpaceMaths.h"
#include "KoCompositeOp.h"


#define NATIVE_MAX_VALUE KoColorSpaceMathsTraits<channels_type>::max
#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::unitValue
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::zeroValue

/**
 * A template base class for all composite op that compose color
 * channels values for colorspaces that do not have an alpha channel.
 * This class looks at the mask value and if the mask value is 0, then
 * the corresponding src and dst pixels will not be composited.
 *
 * @param _compositeOp this class should define a function with the
 * following signature: inline static void composeColorChannels
 */
template<class _CSTraits, class _compositeOp>
class KoCompositeOpNonAlphaBase : public KoCompositeOp {
    typedef typename _CSTraits::channels_type channels_type;
    public:

        KoCompositeOpNonAlphaBase(KoColorSpace * cs, const QString& id, const QString& description)
        : KoCompositeOp(cs, id, description )
        {
        }

    public:

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
            Q_UNUSED( channelFlags );

            channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);
            qint32 pixelSize = colorSpace()->pixelSize();

            while (rows > 0) {
                const channels_type *srcN = reinterpret_cast<const channels_type *>(srcRowStart);
                channels_type *dstN = reinterpret_cast<channels_type *>(dstRowStart);
                const quint8 *mask = maskRowStart;

                qint32 columns = cols;

                while (columns > 0) {

                    channels_type srcAlpha = srcN[_CSTraits::alpha_pos];

                    // Don't blend dst with src if the mask is fully
                    // transparent

                    if (mask != 0) {
                        if (*mask == OPACITY_TRANSPARENT) {
                            mask++;
                            columns--;
                            srcN+=_CSTraits::channels_nb;
                            dstN+=_CSTraits::channels_nb;
                            continue;
                        }
                        mask++;
                    }

                    _compositeOp::composeColorChannels( NATIVE_OPACITY_OPAQUE, srcN, dstN, pixelSize, channelFlags );

                    columns--;
                    srcN+=_CSTraits::channels_nb;
                    dstN+=_CSTraits::channels_nb;
                }

                rows--;
                srcRowStart += srcstride;
                dstRowStart += dststride;
                if(maskRowStart) {
                    maskRowStart += maskstride;
                }
            }
        }

};

#endif
