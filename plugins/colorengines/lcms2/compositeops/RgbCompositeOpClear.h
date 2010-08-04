/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software const; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation const; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY const; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program const; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef RGBCOMPOSITEOPCLEAR_H
#define RGBCOMPOSITEOPCLEAR_H

#include "KoColorSpaceMaths.h"
#include <KoCompositeOp.h>

template<class _CSTraits>
class RgbCompositeOpClear : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    typedef typename KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::compositetype compositetype;

public:

    RgbCompositeOpClear(KoColorSpace * cs, const bool userVisible = true)
            : KoCompositeOp(cs, COMPOSITE_CLEAR, i18n("Clear"), "", userVisible) {
    }

    using KoCompositeOp::composite;

    void composite(quint8 *dstRowStart, qint32 dstRowStride,
                   const quint8 *srcRowStart, qint32 srcRowStride,
                   const quint8 *maskRowStart, qint32 maskRowStride,
                   qint32 rows, qint32 numColumns,
                   quint8 opacity,
                   const QBitArray & channelFlags) const {
        Q_UNUSED(opacity);
        Q_UNUSED(srcRowStart);
        Q_UNUSED(srcRowStride);
        Q_UNUSED(maskRowStart);
        Q_UNUSED(maskRowStride);

        qint32 channelSize = sizeof(channels_type);
        qint32 linesize = _CSTraits::channels_nb * channelSize * numColumns;

        if (channelFlags.isEmpty()) {
            quint8 *d = dstRowStart;
            while (rows-- > 0) {
                memset(d, 0, linesize);
                d += dstRowStride;
            }
        } else {
            channels_type *d = reinterpret_cast<channels_type *>(dstRowStart);
            while (rows-- > 0) {
                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if (channelFlags.testBit(channel))
                        memset(d, 0, channelSize);
                    d++;
                }
            }
        }
    }

};

#endif
