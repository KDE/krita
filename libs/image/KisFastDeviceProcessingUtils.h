/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISFASTDEVICEPROCESSINGUTILS_H
#define KISFASTDEVICEPROCESSINGUTILS_H

#include "kis_random_accessor_ng.h"
#include <KoAlwaysInline.h>

namespace KritaUtils {


/**
 * this is a special helper function for iterating
 * through pixels in an extremely efficient way. One
 * should either pass a functor or a lambda to it.
 */
template <class PixelProcessor>
void processTwoDevices(const QRect &rc,
                       KisRandomConstAccessorSP srcIt,
                       KisRandomAccessorSP dstIt,
                       const int srcPixelSize,
                       const int dstPixelSize,
                       PixelProcessor pixelProcessor)
{
    qint32 dstY = rc.y();
    qint32 rowsRemaining = rc.height();

    while (rowsRemaining > 0) {
        qint32 dstX = rc.x();

        qint32 numContiguousSrcRows = srcIt->numContiguousRows(dstY);
        qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY);
        qint32 rows = std::min({rowsRemaining, numContiguousSrcRows, numContiguousDstRows});

        qint32 columnsRemaining = rc.width();

        while (columnsRemaining > 0) {

            qint32 numContiguousSrcColumns = srcIt->numContiguousColumns(dstX);
            qint32 numContiguousDstColumns = dstIt->numContiguousColumns(dstX);
            qint32 columns = std::min({columnsRemaining, numContiguousSrcColumns, numContiguousDstColumns});

            qint32 dstRowStride = dstIt->rowStride(dstX, dstY);
            qint32 srcRowStride = srcIt->rowStride(dstX, dstY);

            dstIt->moveTo(dstX, dstY);
            srcIt->moveTo(dstX, dstY);

            quint8 *dstRowStart = dstIt->rawData();
            const quint8 *srcRowStart = srcIt->rawDataConst();

            for (int i = 0; i < rows; i++) {
                const quint8 *srcPtr = reinterpret_cast<const quint8*>(srcRowStart);
                quint8 *dstPtr = reinterpret_cast<quint8*>(dstRowStart);

                for (int j = 0; j < columns; j++) {
                    pixelProcessor(srcPtr, dstPtr);

                    srcPtr += srcPixelSize;
                    dstPtr += dstPixelSize;
                }

                srcRowStart += srcRowStride;
                dstRowStart += dstRowStride;
            }

            dstX += columns;
            columnsRemaining -= columns;
        }

        dstY += rows;
        rowsRemaining -= rows;
    }
}
}


#endif // KISFASTDEVICEPROCESSINGUTILS_H
