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

void KisPainter::Private::applyDevice(const QRect &applyRect,
                                      const KisRenderedDab &dab,
                                      KisRandomAccessorSP dstIt,
                                      const KoColorSpace *srcColorSpace,
                                      KoCompositeOp::ParameterInfo &localParamInfo)
{
    const QRect dabRect = dab.realBounds();
    const QRect rc = applyRect & dabRect;

    const int srcPixelSize = srcColorSpace->pixelSize();
    const int dabRowStride = srcPixelSize * dabRect.width();


    qint32 dstY = rc.y();
    qint32 rowsRemaining = rc.height();

    while (rowsRemaining > 0) {
        qint32 dstX = rc.x();

        qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY);
        qint32 rows = qMin(rowsRemaining, numContiguousDstRows);

        qint32 columnsRemaining = rc.width();

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


            const int dabX = dstX - dabRect.x();
            const int dabY = dstY - dabRect.y();

            localParamInfo.srcRowStart   = dab.device->constData() + dabX * srcPixelSize + dabY * dabRowStride;
            localParamInfo.srcRowStride  = dabRowStride;
            localParamInfo.setOpacityAndAverage(dab.opacity, dab.averageOpacity);
            localParamInfo.flow = dab.flow;
            colorSpace->bitBlt(srcColorSpace, localParamInfo, compositeOp, renderingIntent, conversionFlags);

            dstX += columns;
            columnsRemaining -= columns;
        }

        dstY += rows;
        rowsRemaining -= rows;
    }

}

void KisPainter::Private::applyDeviceWithSelection(const QRect &applyRect,
                                                   const KisRenderedDab &dab,
                                                   KisRandomAccessorSP dstIt,
                                                   KisRandomConstAccessorSP maskIt,
                                                   const KoColorSpace *srcColorSpace,
                                                   KoCompositeOp::ParameterInfo &localParamInfo)
{
    const QRect dabRect = dab.realBounds();
    const QRect rc = applyRect & dabRect;

    const int srcPixelSize = srcColorSpace->pixelSize();
    const int dabRowStride = srcPixelSize * dabRect.width();


    qint32 dstY = rc.y();
    qint32 rowsRemaining = rc.height();

    while (rowsRemaining > 0) {
        qint32 dstX = rc.x();

        qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY);
        qint32 numContiguousMaskRows = maskIt->numContiguousRows(dstY);
        qint32 rows = qMin(rowsRemaining, qMin(numContiguousDstRows, numContiguousMaskRows));

        qint32 columnsRemaining = rc.width();

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = dstIt->numContiguousColumns(dstX);
            qint32 numContiguousMaskColumns = maskIt->numContiguousColumns(dstX);
            qint32 columns = qMin(columnsRemaining, qMin(numContiguousDstColumns, numContiguousMaskColumns));

            qint32 dstRowStride = dstIt->rowStride(dstX, dstY);
            qint32 maskRowStride = maskIt->rowStride(dstX, dstY);
            dstIt->moveTo(dstX, dstY);
            maskIt->moveTo(dstX, dstY);

            localParamInfo.dstRowStart   = dstIt->rawData();
            localParamInfo.dstRowStride  = dstRowStride;
            localParamInfo.maskRowStart  = maskIt->rawDataConst();
            localParamInfo.maskRowStride = maskRowStride;
            localParamInfo.rows          = rows;
            localParamInfo.cols          = columns;


            const int dabX = dstX - dabRect.x();
            const int dabY = dstY - dabRect.y();

            localParamInfo.srcRowStart   = dab.device->constData() + dabX * srcPixelSize + dabY * dabRowStride;
            localParamInfo.srcRowStride  = dabRowStride;
            localParamInfo.setOpacityAndAverage(dab.opacity, dab.averageOpacity);
            localParamInfo.flow = dab.flow;
            colorSpace->bitBlt(srcColorSpace, localParamInfo, compositeOp, renderingIntent, conversionFlags);

            dstX += columns;
            columnsRemaining -= columns;
        }

        dstY += rows;
        rowsRemaining -= rows;
    }

}

void KisPainter::bltFixed(const QRect &applyRect, const QList<KisRenderedDab> allSrcDevices)
{
    const KoColorSpace *srcColorSpace = 0;
    QList<KisRenderedDab> devices;
    QRect rc = applyRect;

    if (d->selection) {
        rc &= d->selection->selectedRect();
    }

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

    rc &= totalDevicesRect;

    if (devices.isEmpty() || rc.isEmpty()) return;

    KoCompositeOp::ParameterInfo localParamInfo = d->paramInfo;
    KisRandomAccessorSP dstIt = d->device->createRandomAccessorNG(rc.left(), rc.top());
    KisRandomConstAccessorSP maskIt = d->selection ? d->selection->projection()->createRandomConstAccessorNG(rc.left(), rc.top()) : 0;

    if (maskIt) {
        Q_FOREACH (const KisRenderedDab &dab, devices) {
            d->applyDeviceWithSelection(rc, dab, dstIt, maskIt, srcColorSpace, localParamInfo);
        }
    } else {
        Q_FOREACH (const KisRenderedDab &dab, devices) {
            d->applyDevice(rc, dab, dstIt, srcColorSpace, localParamInfo);
        }
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

