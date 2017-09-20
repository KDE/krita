/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_painter.h"
#include "kis_painter_p.h"

#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_random_accessor_ng.h"
#include "KisRenderedDab.h"

void KisPainter::Private::collectRowDevices(int x1, int x2, int y, int y2,
                                            const QList<KisRenderedDab> allDevices,
                                            QList<KisRenderedDab> *rowDevices,
                                            int *numContiguousRows)
{
    *numContiguousRows = y2 - y + 1;
    rowDevices->clear();

    for (auto it = allDevices.cbegin(); it != allDevices.cend(); ++it) {
        const QRect rc = it->realBounds();

        if (rc.left() > x2 || rc.right() < x1 ||
            rc.top() > y || rc.bottom() < y) {

            const int distance = rc.top() - y;
            if (distance > 0 && distance < *numContiguousRows) {
                *numContiguousRows = distance;
            }

            continue;
        }

        *numContiguousRows = qMin(*numContiguousRows, rc.bottom() - y + 1);
        *rowDevices << *it;
    }
}

struct KisPainter::Private::ChunkDescriptor {
    quint8 *ptr;
    int rowStride;

    float opacity = 1.0;
    float averageOpacity = 1.0;
    float flow = 1.0;
};

void KisPainter::Private::collectChunks(int x, int x2, int y,
                                        QList<KisRenderedDab> rowDevices,
                                        QList<ChunkDescriptor> *chunks,
                                        int *numContiguosColumns)
{
    *numContiguosColumns = x2 - x + 1;
    chunks->erase(chunks->begin(), chunks->end());

    for (auto it = rowDevices.cbegin(); it != rowDevices.cend(); ++it) {
        const QRect rc = it->realBounds();

        if (rc.left() > x || rc.right() < x) {
            const int distance = rc.left() - x;
            if (distance > 0 && distance < *numContiguosColumns) {
                *numContiguosColumns = distance;
            }

            continue;
        }

        *numContiguosColumns = qMin(*numContiguosColumns,
                                    rc.right() - x + 1);

        const int pixelSize = it->device->pixelSize();

        ChunkDescriptor chunk;
        chunk.opacity = it->opacity;
        chunk.averageOpacity = it->averageOpacity;
        chunk.flow = it->flow;
        chunk.rowStride = rc.width() * pixelSize;
        chunk.ptr =
            it->device->data() +
            chunk.rowStride * (y - rc.top()) +
            pixelSize * (x - rc.left());

        *chunks << chunk;
    }
}

void KisPainter::Private::applyChunks(int x, int y, int width, int height,
                                      KisRandomAccessorSP dstIt,
                                      const QList<ChunkDescriptor> &chunks,
                                      const KoColorSpace *srcColorSpace,
                                      KoCompositeOp::ParameterInfo &localParamInfo)
{
    const int srcPixelSize = srcColorSpace->pixelSize();
    QList<ChunkDescriptor> savedChunks = chunks;

    qint32 dstY = y;
    qint32 rowsRemaining = height;

    while (rowsRemaining > 0) {
        qint32 dstX = x;

        qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY);
        qint32 rows = qMin(rowsRemaining, numContiguousDstRows);

        qint32 columnsRemaining = width;

        QList<ChunkDescriptor> rowChunks = savedChunks;

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = dstIt->numContiguousColumns(dstX);
            qint32 columns = qMin(numContiguousDstColumns, columnsRemaining);

            qint32 dstRowStride = dstIt->rowStride(dstX, dstY);
            dstIt->moveTo(dstX, dstY);

            localParamInfo.dstRowStart   = dstIt->rawData();
            localParamInfo.dstRowStride  = dstRowStride;
            localParamInfo.maskRowStart  = 0;
            localParamInfo.maskRowStride = 0;
            localParamInfo.rows          = rows;
            localParamInfo.cols          = columns;

            const int srcColumnStep = srcPixelSize * columns;

            for (auto it = rowChunks.begin(); it != rowChunks.end(); ++it) {
                localParamInfo.srcRowStart   = it->ptr;
                localParamInfo.srcRowStride  = it->rowStride;
                localParamInfo.setOpacityAndAverage(it->opacity, it->averageOpacity);
                localParamInfo.flow = it->flow;
                colorSpace->bitBlt(srcColorSpace, localParamInfo, compositeOp, renderingIntent, conversionFlags);

                it->ptr += srcColumnStep;
            }

            dstX += columns;
            columnsRemaining -= columns;
        }

        for (auto it = savedChunks.begin(); it != savedChunks.end(); ++it) {
            it->ptr += it->rowStride * rows;
        }

        dstY += rows;
        rowsRemaining -= rows;
    }

}


void KisPainter::bltFixed(const QRect &rc, const QList<KisRenderedDab> allSrcDevices)
{
    const KoColorSpace *srcColorSpace = 0;
    QList<KisRenderedDab> devices;
    QRect totalDevicesRect;

    Q_FOREACH (const KisRenderedDab &dab, allSrcDevices) {
        if (rc.intersects(dab.realBounds())) {
            devices.append(dab);
            totalDevicesRect |= dab.realBounds();
        }

        if (!srcColorSpace) {
            srcColorSpace = dab.device->colorSpace();
        } else {
            KIS_SAFE_ASSERT_RECOVER_RETURN(*srcColorSpace == *dab.device->colorSpace());
        }
    }

    if (devices.isEmpty() || !totalDevicesRect.intersects(rc)) return;

    KoCompositeOp::ParameterInfo localParamInfo = d->paramInfo;
    KisRandomAccessorSP dstIt = d->device->createRandomAccessorNG(rc.left(), rc.top());

    int row = rc.top();
    QList<KisRenderedDab> rowDevices;
    int numContiguousRowsInDevices = 0;

    while (row <= rc.bottom()) {
        d->collectRowDevices(rc.left(), rc.right(), row, rc.bottom(),
                             devices,
                             &rowDevices, &numContiguousRowsInDevices);

        if (!rowDevices.isEmpty()) {
            int column = rc.left();
            int numContiguousColumnsInDevices = 0;
            QList<Private::ChunkDescriptor> chunks;

            while (column <= rc.right()) {
                d->collectChunks(column, rc.right(), row, rowDevices,
                                 &chunks, &numContiguousColumnsInDevices);

                if (!chunks.isEmpty()) {
                    d->applyChunks(column, row,
                                   numContiguousColumnsInDevices, numContiguousRowsInDevices,
                                   dstIt,
                                   chunks,
                                   srcColorSpace,
                                   localParamInfo);
                }

                column += numContiguousColumnsInDevices;
            }

        }

        row += numContiguousRowsInDevices;
    }


#if 0
    // the code above does basically the same thing as this one,
    // but more efficiently :)

    Q_FOREACH (KisFixedPaintDeviceSP dev, devices) {
        const QRect copyRect = dev->bounds() & rc;
        if (copyRect.isEmpty()) continue;

        bltFixed(copyRect.topLeft(), dev, copyRect);
    }
#endif
}

