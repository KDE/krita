/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara Toloza <pentalis@gmail.com>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <cmath>
#include <climits>
#include <strings.h>

#include <QImage>
#include <QRect>
#include <QString>
#include <QStringList>
#include <kundo2command.h>

#include <kis_debug.h>
#include <klocale.h>
#include <kglobal.h>

#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoCompositeOp.h>

#include "kis_image.h"
#include "filter/kis_filter.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_pixel_selection.h"
#include "kis_paint_information.h"
#include "kis_paintop_registry.h"
#include "kis_perspective_math.h"
#include "tiles3/kis_random_accessor.h"

// Maximum distance from a Bezier control point to the line through the start
// and end points for the curve to be considered flat.
#define BEZIER_FLATNESS_THRESHOLD 0.5

struct KisPainter::Private {
    KisPaintDeviceSP            device;
    KisSelectionSP              selection;
    KisTransaction*             transaction;
    KoUpdater*                  progressUpdater;

    QVector<QRect>              dirtyRects;
    KisPaintOp*                 paintOp;
    QRect                       bounds;
    KoColor                     paintColor;
    KoColor                     backgroundColor;
    KoColor                     fillColor;
    const KisFilterConfiguration* generator;
    KisPaintLayer*              sourceLayer;
    FillStyle                   fillStyle;
    StrokeStyle                 strokeStyle;
    bool                        antiAliasPolygonFill;
    const KisPattern*           pattern;
    QPointF                     duplicateOffset;
    quint32                     pixelSize;
    const KoColorSpace*         colorSpace;
    KoColorProfile*             profile;
    const KoCompositeOp*        compositeOp;
    const KoAbstractGradient*   gradient;
    KisPaintOpPresetSP          paintOpPreset;
    QImage                      polygonMaskImage;
    QPainter*                   maskPainter;
    KisFillPainter*             fillPainter;
    KisPaintDeviceSP            polygon;
    qint32                      maskImageWidth;
    qint32                      maskImageHeight;
    QPointF                     axisCenter;
    bool                        mirrorHorizontaly;
    bool                        mirrorVerticaly;
    KoCompositeOp::ParameterInfo paramInfo;
    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;
};

KisPainter::KisPainter()
        : d(new Private)
{
    init();
}

KisPainter::KisPainter(KisPaintDeviceSP device)
        : d(new Private)
{
    init();
    Q_ASSERT(device);
    begin(device);
}

KisPainter::KisPainter(KisPaintDeviceSP device, KisSelectionSP selection)
        : d(new Private)
{
    init();
    Q_ASSERT(device);
    begin(device);
    d->selection = selection;
}

void KisPainter::init()
{
    d->selection = 0 ;
    d->transaction = 0;
    d->paintOp = 0;
    d->pattern = 0;
    d->sourceLayer = 0;
    d->fillStyle = FillStyleNone;
    d->strokeStyle = StrokeStyleBrush;
    d->antiAliasPolygonFill = true;
    d->bounds = QRect();
    d->progressUpdater = 0;
    d->gradient = 0;
    d->maskPainter = 0;
    d->fillPainter = 0;
    d->maskImageWidth = 255;
    d->maskImageHeight = 255;
    d->mirrorHorizontaly = false;
    d->mirrorVerticaly = false;
    d->paramInfo.opacity = 1.0f;
    d->paramInfo.flow = 1.0f;
    d->renderingIntent = KoColorConversionTransformation::IntentPerceptual;
    d->conversionFlags = KoColorConversionTransformation::Empty;
}

KisPainter::~KisPainter()
{
    // TODO: Maybe, don't be that strict?
    // deleteTransaction();
    end();

    delete d->paintOp;
    delete d->maskPainter;
    delete d->fillPainter;
    delete d;
}

void KisPainter::begin(KisPaintDeviceSP device)
{
    begin(device, d->selection);
}

void KisPainter::begin(KisPaintDeviceSP device, KisSelectionSP selection)
{
    if (!device) return;
    d->selection = selection;
    Q_ASSERT(device->colorSpace());

    end();

    d->device = device;
    d->colorSpace = device->colorSpace();
    d->compositeOp = d->colorSpace->compositeOp(COMPOSITE_OVER);
    d->pixelSize = device->pixelSize();
}

void KisPainter::end()
{
    Q_ASSERT_X(!d->transaction, "KisPainter::end()",
               "end() was called for the painter having a transaction. "
               "Please use end/deleteTransaction() instead");
}

void KisPainter::beginTransaction(const QString& transactionName)
{
    Q_ASSERT_X(!d->transaction, "KisPainter::beginTransaction()",
               "You asked for a new transaction while still having "
               "another one. Please finish the first one with "
               "end/deleteTransaction() first");

    d->transaction = new KisTransaction(transactionName, d->device);
    Q_CHECK_PTR(d->transaction);
}

QString KisPainter::transactionText()
{
    Q_ASSERT_X(d->transaction, "KisPainter::transactionText()",
               "No transaction is in progress");

    return d->transaction->text();
}

void KisPainter::revertTransaction()
{
    Q_ASSERT_X(d->transaction, "KisPainter::revertTransaction()",
               "No transaction is in progress");

    d->transaction->revert();
    delete d->transaction;
    d->transaction = 0;
}

void KisPainter::endTransaction(KisUndoAdapter *undoAdapter)
{
    Q_ASSERT_X(d->transaction, "KisPainter::endTransaction()",
               "No transaction is in progress");

    d->transaction->commit(undoAdapter);
    delete d->transaction;
    d->transaction = 0;
}

void KisPainter::endTransaction(KisPostExecutionUndoAdapter *undoAdapter)
{
    Q_ASSERT_X(d->transaction, "KisPainter::endTransaction()",
               "No transaction is in progress");

    d->transaction->commit(undoAdapter);
    delete d->transaction;
    d->transaction = 0;
}

void KisPainter::deleteTransaction()
{
    if (!d->transaction) return;

    delete d->transaction;
    d->transaction = 0;
}

void KisPainter::putTransaction(KisTransaction* transaction)
{
    Q_ASSERT_X(!d->transaction, "KisPainter::putTransaction()",
               "You asked for a new transaction while still having "
               "another one. Please finish the first one with "
               "end/deleteTransaction() first");

    d->transaction = transaction;
}

KisTransaction* KisPainter::takeTransaction()
{
    Q_ASSERT_X(d->transaction, "KisPainter::takeTransaction()",
               "No transaction is in progress");
    KisTransaction *temp = d->transaction;
    d->transaction = 0;
    return temp;
}

QVector<QRect> KisPainter::takeDirtyRegion()
{
    QVector<QRect> vrect = d->dirtyRects;
    d->dirtyRects.clear();
    return vrect;
}


void KisPainter::addDirtyRect(const QRect & rc)
{
    QRect r = rc.normalized();
    if (r.isValid()) {
        d->dirtyRects.append(rc);
    }
}

void KisPainter::bitBltWithFixedSelection(qint32 dstX, qint32 dstY,
                                          const KisPaintDeviceSP srcDev,
                                          const KisFixedPaintDeviceSP selection,
                                          qint32 selX, qint32 selY,
                                          qint32 srcX, qint32 srcY,
                                          quint32 srcWidth, quint32 srcHeight)
{
    // TODO: get selX and selY working as intended

    /* This check for nonsense ought to be a Q_ASSERT. However, when paintops are just
    initializing they perform some dummy passes with those parameters, and it must not crash */
    if (srcWidth == 0 || srcHeight == 0) return;
    if (srcDev.isNull()) return;
    if (d->device.isNull()) return;

    // TODO: What purpose has checking if src and this have the same pixel size?
    Q_ASSERT(srcDev->pixelSize() == d->pixelSize);
    // Check that selection has an alpha colorspace, crash if false
    Q_ASSERT(selection->colorSpace() == KoColorSpaceRegistry::instance()->alpha8());

    QRect srcRect = QRect(srcX, srcY, srcWidth, srcHeight);
    QRect selRect = QRect(selX, selY, srcWidth, srcHeight);

    /* Trying to read outside a KisFixedPaintDevice is inherently wrong and shouldn't be done,
    so crash if someone attempts to do this. Don't resize YET as it would obfuscate the mistake. */
    Q_ASSERT(selection->bounds().contains(selRect));
    Q_UNUSED(selRect); // only used by the above Q_ASSERT

    /* KisPaintDevice is a tricky beast, and it can easily have unpredictable exactBounds() or extent()
    in limit cases (for example, when brushes become too small in a brush engine). That's why there's no
    Q_ASSERT to check correlation between srcRect and those bounds.
    Below lies a speed optimization that shrinks srcRect only when srcDev is smaller than it. This is no
    problem and without it there would be no illegal read from memory since reading out the bounds of a
    KisPaintDevice gives the default pixel.*/

    /* In case of COMPOSITE_COPY restricting bitBlt to extent can
    have unexpected behavior since it would reduce the area that
    is copied (Read below). */
    if (d->compositeOp->id() != COMPOSITE_COPY) {
        /* If srcDev->extent() (the area of the tiles containing srcDev)
        is smaller than srcRect, then shrink srcRect to that size. This
        is done as a speed optimization, useful for stack recomposition
        in KisImage. srcRect won't grow if srcDev->extent() is larger. */
        srcRect &= srcDev->extent();

        if (srcRect.isEmpty()) return;

        // Readjust the function paramenters to the new dimensions.
        dstX += srcRect.x() - srcX;    // This will only add, not subtract
        dstY += srcRect.y() - srcY;    // Idem
        srcX = srcRect.x();
        srcY = srcRect.y();
        srcWidth = srcRect.width();
        srcHeight = srcRect.height();
    }

    /* Create an intermediate byte array to hold information before it is written
    to the current paint device (d->device) */
    quint8* dstBytes = new quint8[srcWidth * srcHeight * d->device->pixelSize()];
    d->device->readBytes(dstBytes, dstX, dstY, srcWidth, srcHeight);

    // Copy the relevant bytes of raw data from srcDev
    quint8* srcBytes = new quint8[srcWidth * srcHeight * d->device->pixelSize()];
    srcDev->readBytes(srcBytes, srcX, srcY, srcWidth, srcHeight);

    /*
     * This checks whether there is nothing selected.
     */
    if (!d->selection) {
        /* As there's nothing selected, blit to dstBytes (intermediary bit array),
          ignoring d->selection (the user selection)*/
        d->paramInfo.dstRowStart   = dstBytes;
        d->paramInfo.dstRowStride  = srcWidth * d->device->pixelSize();
        d->paramInfo.srcRowStart   = srcBytes;
        d->paramInfo.srcRowStride  = srcWidth * srcDev->pixelSize();
        d->paramInfo.maskRowStart  = selection->data() + selX;
        d->paramInfo.maskRowStride = srcWidth * selection->pixelSize();
        d->paramInfo.rows          = srcHeight;
        d->paramInfo.cols          = srcWidth;
        d->colorSpace->bitBlt(srcDev->colorSpace(), d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);
    }
    else {
        /* Read the user selection (d->selection) bytes into an array, ready
        to merge in the next block*/
        quint32 totalBytes = srcWidth * srcHeight * selection->pixelSize();
        quint8* mergedSelectionBytes = new quint8[ totalBytes ];
        d->selection->projection()->readBytes(mergedSelectionBytes, dstX, dstY, srcWidth, srcHeight);

        // Merge selections here by multiplying them - compositeOP(COMPOSITE_MULT)
        d->paramInfo.dstRowStart   = mergedSelectionBytes;
        d->paramInfo.dstRowStride  = srcWidth * selection->pixelSize();
        d->paramInfo.srcRowStart   = selection->data() + selX;
        d->paramInfo.srcRowStride  = srcWidth * selection->pixelSize();
        d->paramInfo.maskRowStart  = 0;
        d->paramInfo.maskRowStride = 0;
        d->paramInfo.rows          = srcHeight;
        d->paramInfo.cols          = srcWidth;
        KoColorSpaceRegistry::instance()->alpha8()->compositeOp(COMPOSITE_MULT)->composite(d->paramInfo);

        // Blit to dstBytes (intermediary bit array)
        d->paramInfo.dstRowStart   = dstBytes;
        d->paramInfo.dstRowStride  = srcWidth * d->device->pixelSize();
        d->paramInfo.srcRowStart   = srcBytes;
        d->paramInfo.srcRowStride  = srcWidth * srcDev->pixelSize();
        d->paramInfo.maskRowStart  = mergedSelectionBytes;
        d->paramInfo.maskRowStride = srcWidth * selection->pixelSize();
        d->colorSpace->bitBlt(d->colorSpace, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);
        delete[] mergedSelectionBytes;
    }

    d->device->writeBytes(dstBytes, dstX, dstY, srcWidth, srcHeight);

    delete[] dstBytes;
    delete[] srcBytes;

    addDirtyRect(QRect(dstX, dstY, srcWidth, srcHeight));
}


void KisPainter::bitBltWithFixedSelection(qint32 dstX, qint32 dstY,
                                          const KisPaintDeviceSP srcDev,
                                          const KisFixedPaintDeviceSP selection,
                                          quint32 srcWidth, quint32 srcHeight)
{
    bitBltWithFixedSelection(dstX, dstY, srcDev, selection, 0, 0, 0, 0, srcWidth, srcHeight);
}

template <bool useOldSrcData>
void KisPainter::bitBltImpl(qint32 dstX, qint32 dstY,
                            const KisPaintDeviceSP srcDev,
                            qint32 srcX, qint32 srcY,
                            qint32 srcWidth, qint32 srcHeight)
{
    /* This check for nonsense ought to be a Q_ASSERT. However, when paintops are just
    initializing they perform some dummy passes with those parameters, and it must not crash */
    if (srcWidth == 0 || srcHeight == 0) return;
    if (srcDev.isNull()) return;
    if (d->device.isNull()) return;

    QRect srcRect = QRect(srcX, srcY, srcWidth, srcHeight);

    if (d->compositeOp->id() == COMPOSITE_COPY) {
        if(!d->selection && srcX == dstX && srcY == dstY && d->device->fastBitBltPossible(srcDev)) {

            if(useOldSrcData) {
                d->device->fastBitBltOldData(srcDev, srcRect);
            } else {
                d->device->fastBitBlt(srcDev, srcRect);
            }

            addDirtyRect(srcRect);
            return;
        }
    }
    else {
        /* In case of COMPOSITE_COPY restricting bitBlt to extent can
        have unexpected behavior since it would reduce the area that
        is copied (Read below). */
        if (d->compositeOp->id() != COMPOSITE_COPY) {
            /* If srcDev->extent() (the area of the tiles containing srcDev)
            is smaller than srcRect, then shrink srcRect to that size. This
            is done as a speed optimization, useful for stack recomposition
            in KisImage. srcRect won't grow if srcDev->extent() is larger. */
            srcRect &= srcDev->extent();

            if (srcRect.isEmpty()) return;

            // Readjust the function paramenters to the new dimensions.
            dstX += srcRect.x() - srcX;    // This will only add, not subtract
            dstY += srcRect.y() - srcY;    // Idem
            srcX = srcRect.x();
            srcY = srcRect.y();
            srcWidth = srcRect.width();
            srcHeight = srcRect.height();
        }
    }
    const KoColorSpace * srcCs = srcDev->colorSpace();

    qint32 dstY_ = dstY;
    qint32 srcY_ = srcY;
    qint32 rowsRemaining = srcHeight;

    // Read below
    KisRandomConstAccessorSP srcIt = srcDev->createRandomConstAccessorNG(srcX, srcY);
    KisRandomAccessorSP dstIt = d->device->createRandomAccessorNG(dstX, dstY);

    /* Here be a huge block of verbose code that does roughly the same than
    the other bit blit operations. This one is longer than the rest in an effort to
    optimize speed and memory use */
    if (d->selection) {
        KisPaintDeviceSP selectionProjection = d->selection->projection();
        KisRandomConstAccessorSP maskIt = selectionProjection->createRandomConstAccessorNG(dstX, dstY);

        while (rowsRemaining > 0) {

            qint32 dstX_ = dstX;
            qint32 srcX_ = srcX;
            qint32 columnsRemaining = srcWidth;
            qint32 numContiguousDstRows = d->device->numContiguousRows(dstY_, dstX_, dstX_ + srcWidth - 1);
            qint32 numContiguousSrcRows = srcDev->numContiguousRows(srcY_, srcX_, srcX_ + srcWidth - 1);
            qint32 numContiguousSelRows = selectionProjection->numContiguousRows(srcY_, srcX_, srcX_ + srcWidth - 1);

            qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
            rows = qMin(rows, numContiguousSelRows);
            rows = qMin(rows, rowsRemaining);

            while (columnsRemaining > 0) {

                qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX_, dstY_, dstY_ + rows - 1);
                qint32 numContiguousSrcColumns = srcDev->numContiguousColumns(srcX_, srcY_, srcY_ + rows - 1);
                qint32 numContiguousSelColumns = selectionProjection->numContiguousColumns(srcX_, srcY_, srcY_ + rows - 1);

                qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
                columns = qMin(columns, numContiguousSelColumns);
                columns = qMin(columns, columnsRemaining);

                qint32 srcRowStride = srcDev->rowStride(srcX_, srcY_);
                srcIt->moveTo(srcX_, srcY_);

                qint32 dstRowStride = d->device->rowStride(dstX_, dstY_);
                dstIt->moveTo(dstX_, dstY_);

                qint32 maskRowStride = selectionProjection->rowStride(dstX_, dstY_);
                maskIt->moveTo(dstX_, dstY_);

                d->paramInfo.dstRowStart   = dstIt->rawData();
                d->paramInfo.dstRowStride  = dstRowStride;
                // if we don't use the oldRawData, we need to access the rawData of the source device.
                d->paramInfo.srcRowStart   = useOldSrcData ? srcIt->oldRawData() : static_cast<KisRandomAccessor2*>(srcIt.data())->rawData();
                d->paramInfo.srcRowStride  = srcRowStride;
                d->paramInfo.maskRowStart  = static_cast<KisRandomAccessor2*>(maskIt.data())->rawData();
                d->paramInfo.maskRowStride = maskRowStride;
                d->paramInfo.rows          = rows;
                d->paramInfo.cols          = columns;
                d->colorSpace->bitBlt(srcCs, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);

                srcX_ += columns;
                dstX_ += columns;
                columnsRemaining -= columns;
            }

            srcY_ += rows;
            dstY_ += rows;
            rowsRemaining -= rows;
        }
    }
    else {

        while (rowsRemaining > 0) {

            qint32 dstX_ = dstX;
            qint32 srcX_ = srcX;
            qint32 columnsRemaining = srcWidth;
            qint32 numContiguousDstRows = d->device->numContiguousRows(dstY_, dstX_, dstX_ + srcWidth - 1);
            qint32 numContiguousSrcRows = srcDev->numContiguousRows(srcY_, srcX_, srcX_ + srcWidth - 1);

            qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
            rows = qMin(rows, rowsRemaining);

            while (columnsRemaining > 0) {

                qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX_, dstY_, dstY_ + rows - 1);
                qint32 numContiguousSrcColumns = srcDev->numContiguousColumns(srcX_, srcY_, srcY_ + rows - 1);

                qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
                columns = qMin(columns, columnsRemaining);

                qint32 srcRowStride = srcDev->rowStride(srcX_, srcY_);
                srcIt->moveTo(srcX_, srcY_);

                qint32 dstRowStride = d->device->rowStride(dstX_, dstY_);
                dstIt->moveTo(dstX_, dstY_);

                d->paramInfo.dstRowStart   = dstIt->rawData();
                d->paramInfo.dstRowStride  = dstRowStride;
                // if we don't use the oldRawData, we need to access the rawData of the source device.
                d->paramInfo.srcRowStart   = useOldSrcData ? srcIt->oldRawData() : static_cast<KisRandomAccessor2*>(srcIt.data())->rawData();
                d->paramInfo.srcRowStride  = srcRowStride;
                d->paramInfo.maskRowStart  = 0;
                d->paramInfo.maskRowStride = 0;
                d->paramInfo.rows          = rows;
                d->paramInfo.cols          = columns;
                d->colorSpace->bitBlt(srcCs, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);

                srcX_ += columns;
                dstX_ += columns;
                columnsRemaining -= columns;
            }

            srcY_ += rows;
            dstY_ += rows;
            rowsRemaining -= rows;
        }
    }

    addDirtyRect(QRect(dstX, dstY, srcWidth, srcHeight));

}

void KisPainter::bitBlt(qint32 dstX, qint32 dstY,
                        const KisPaintDeviceSP srcDev,
                        qint32 srcX, qint32 srcY,
                        qint32 srcWidth, qint32 srcHeight)
{
    bitBltImpl<false>(dstX, dstY, srcDev, srcX, srcY, srcWidth, srcHeight);
}


void KisPainter::bitBlt(const QPoint & pos, const KisPaintDeviceSP srcDev, const QRect & srcRect)
{
    bitBlt(pos.x(), pos.y(), srcDev, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}

void KisPainter::bitBltOldData(qint32 dstX, qint32 dstY,
                               const KisPaintDeviceSP srcDev,
                               qint32 srcX, qint32 srcY,
                               qint32 srcWidth, qint32 srcHeight)
{
    bitBltImpl<true>(dstX, dstY, srcDev, srcX, srcY, srcWidth, srcHeight);
}


void KisPainter::bitBltOldData(const QPoint & pos, const KisPaintDeviceSP srcDev, const QRect & srcRect)
{
    bitBltOldData(pos.x(), pos.y(), srcDev, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}


void KisPainter::fill(qint32 x, qint32 y, qint32 width, qint32 height, const KoColor& color)
{
    /* This check for nonsense ought to be a Q_ASSERT. However, when paintops are just
     * initializing they perform some dummy passes with those parameters, and it must not crash */
    if(width == 0 || height == 0 || d->device.isNull())
        return;

    KoColor srcColor(color, d->colorSpace);
    qint32  dstY          = y;
    qint32  rowsRemaining = height;

    KisRandomAccessorSP dstIt = d->device->createRandomAccessorNG(x, y);

    if(d->selection) {
        KisPaintDeviceSP selectionProjection = d->selection->projection();
        KisRandomConstAccessorSP maskIt = selectionProjection->createRandomConstAccessorNG(x, y);

        while(rowsRemaining > 0) {

            qint32 dstX                 = x;
            qint32 columnsRemaining     = width;
            qint32 numContiguousDstRows = d->device->numContiguousRows(dstY, dstX, dstX+width-1);
            qint32 numContiguousSelRows = selectionProjection->numContiguousRows(dstY, dstX, dstX+width-1);
            qint32 rows = qMin(numContiguousDstRows, numContiguousSelRows);
            rows = qMin(rows, rowsRemaining);

            while (columnsRemaining > 0) {

                qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX, dstY, dstY+rows-1);
                qint32 numContiguousSelColumns = selectionProjection->numContiguousColumns(dstX, dstY, dstY+rows-1);

                qint32 columns = qMin(numContiguousDstColumns, numContiguousSelColumns);
                columns = qMin(columns, columnsRemaining);

                qint32 dstRowStride = d->device->rowStride(dstX, dstY);
                dstIt->moveTo(dstX, dstY);
                qint32 maskRowStride = selectionProjection->rowStride(dstX, dstY);

                maskIt->moveTo(dstX, dstY);

                d->paramInfo.dstRowStart   = dstIt->rawData();
                d->paramInfo.dstRowStride  = dstRowStride;
                d->paramInfo.srcRowStart   = srcColor.data();
                d->paramInfo.srcRowStride  = 0; // srcRowStride is set to zero to use the compositeOp with only a single color pixel
                d->paramInfo.maskRowStart  = maskIt->oldRawData();
                d->paramInfo.maskRowStride = maskRowStride;
                d->paramInfo.rows          = rows;
                d->paramInfo.cols          = columns;
                d->colorSpace->bitBlt(d->colorSpace, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);

                dstX             += columns;
                columnsRemaining -= columns;
            }

            dstY          += rows;
            rowsRemaining -= rows;
        }
    }
    else {

        while(rowsRemaining > 0) {

            qint32 dstX                 = x;
            qint32 columnsRemaining     = width;
            qint32 numContiguousDstRows = d->device->numContiguousRows(dstY, dstX, dstX+width-1);
            qint32 rows                 = qMin(numContiguousDstRows, rowsRemaining);

            while(columnsRemaining > 0) {

                qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX, dstY, dstY+rows-1);
                qint32 columns                 = qMin(numContiguousDstColumns, columnsRemaining);
                qint32 dstRowStride            = d->device->rowStride(dstX, dstY);
                dstIt->moveTo(dstX, dstY);

                d->paramInfo.dstRowStart   = dstIt->rawData();
                d->paramInfo.dstRowStride  = dstRowStride;
                d->paramInfo.srcRowStart   = srcColor.data();
                d->paramInfo.srcRowStride  = 0; // srcRowStride is set to zero to use the compositeOp with only a single color pixel
                d->paramInfo.maskRowStart  = 0;
                d->paramInfo.maskRowStride = 0;
                d->paramInfo.rows          = rows;
                d->paramInfo.cols          = columns;
                d->colorSpace->bitBlt(d->colorSpace, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);

                dstX             += columns;
                columnsRemaining -= columns;
            }

            dstY          += rows;
            rowsRemaining -= rows;
        }
    }

    addDirtyRect(QRect(x, y, width, height));
}


void KisPainter::bltFixed(qint32 dstX, qint32 dstY,
                          const KisFixedPaintDeviceSP srcDev,
                          qint32 srcX, qint32 srcY,
                          qint32 srcWidth, qint32 srcHeight)
{
    /* This check for nonsense ought to be a Q_ASSERT. However, when paintops are just
    initializing they perform some dummy passes with those parameters, and it must not crash */
    if (srcWidth == 0 || srcHeight == 0) return;
    if (srcDev.isNull()) return;
    if (d->device.isNull()) return;

    // TODO: What purpose has checking if src and this have the same pixel size?
    Q_ASSERT(srcDev->pixelSize() == d->pixelSize);

    QRect srcRect = QRect(srcX, srcY, srcWidth, srcHeight);

    /* Trying to read outside a KisFixedPaintDevice is inherently wrong and shouldn't be done,
    so crash if someone attempts to do this. Don't resize as it would obfuscate the mistake. */
    Q_ASSERT(srcDev->bounds().contains(srcRect));
    Q_UNUSED(srcRect); // only used in above assertion

    // Convenient renaming
    const KoColorSpace * srcCs = d->colorSpace;

    /* Create an intermediate byte array to hold information before it is written
    to the current paint device (aka: d->device) */
    quint8* dstBytes = new quint8[srcWidth * srcHeight * d->device->pixelSize()];
    d->device->readBytes(dstBytes, dstX, dstY, srcWidth, srcHeight);

    d->paramInfo.dstRowStart   = dstBytes;
    d->paramInfo.dstRowStride  = srcWidth * d->device->pixelSize();
    d->paramInfo.srcRowStart   = srcDev->data() + srcX;
    d->paramInfo.srcRowStride  = srcDev->bounds().width() * srcDev->pixelSize();
    d->paramInfo.maskRowStart  = 0;
    d->paramInfo.maskRowStride = 0;
    d->paramInfo.rows          = srcHeight;
    d->paramInfo.cols          = srcWidth;

    if (d->selection) {
        /* d->selection is a KisPaintDevice, so first a readBytes is performed to
        get the area of interest... */
        KisPaintDeviceSP selectionProjection = d->selection->projection();
        quint8* selBytes = new quint8[srcWidth * srcHeight * selectionProjection->pixelSize()];
        selectionProjection->readBytes(selBytes, dstX, dstY, srcWidth, srcHeight);
        d->paramInfo.maskRowStart  = selBytes;
        d->paramInfo.maskRowStride = srcWidth * selectionProjection->pixelSize();
    }

    // ...and then blit.
    d->colorSpace->bitBlt(srcCs, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);
    d->device->writeBytes(dstBytes, dstX, dstY, srcWidth, srcHeight);

    delete[] d->paramInfo.maskRowStart;
    delete[] dstBytes;

    addDirtyRect(QRect(dstX, dstY, srcWidth, srcHeight));
}

void KisPainter::bltFixed(const QPoint & pos, const KisFixedPaintDeviceSP srcDev, const QRect & srcRect)
{
    bltFixed(pos.x(), pos.y(), srcDev, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}

void KisPainter::bltFixedWithFixedSelection(qint32 dstX, qint32 dstY,
                                            const KisFixedPaintDeviceSP srcDev,
                                            const KisFixedPaintDeviceSP selection,
                                            qint32 selX, qint32 selY,
                                            qint32 srcX, qint32 srcY,
                                            quint32 srcWidth, quint32 srcHeight)
{
    // TODO: get selX and selY working as intended

    /* This check for nonsense ought to be a Q_ASSERT. However, when paintops are just
    initializing they perform some dummy passes with those parameters, and it must not crash */
    if (srcWidth == 0 || srcHeight == 0) return;
    if (srcDev.isNull()) return;
    if (d->device.isNull()) return;

    // TODO: What purpose has checking if src and this have the same pixel size?
    Q_ASSERT(srcDev->pixelSize() == d->pixelSize);
    // Check that selection has an alpha colorspace, crash if false
    Q_ASSERT(selection->colorSpace() == KoColorSpaceRegistry::instance()->alpha8());

    QRect srcRect = QRect(srcX, srcY, srcWidth, srcHeight);
    QRect selRect = QRect(selX, selY, srcWidth, srcHeight);

    /* Trying to read outside a KisFixedPaintDevice is inherently wrong and shouldn't be done,
    so crash if someone attempts to do this. Don't resize as it would obfuscate the mistake. */
    Q_ASSERT(srcDev->bounds().contains(srcRect));
    Q_UNUSED(srcRect); // only used in above assertion
    Q_ASSERT(selection->bounds().contains(selRect));
    Q_UNUSED(selRect); // only used in above assertion

    /* Create an intermediate byte array to hold information before it is written
    to the current paint device (aka: d->device) */
    quint8* dstBytes = new quint8[srcWidth * srcHeight * d->device->pixelSize()];
    d->device->readBytes(dstBytes, dstX, dstY, srcWidth, srcHeight);

    if (!d->selection) {
        /* As there's nothing selected, blit to dstBytes (intermediary bit array),
          ignoring d->selection (the user selection)*/
        d->paramInfo.dstRowStart   = dstBytes;
        d->paramInfo.dstRowStride  = srcWidth * d->device->pixelSize();
        d->paramInfo.srcRowStart   = srcDev->data() + srcX;
        d->paramInfo.srcRowStride  = srcDev->bounds().width() * srcDev->pixelSize();
        d->paramInfo.maskRowStart  = selection->data() + selX;
        d->paramInfo.maskRowStride = srcWidth * selection->pixelSize();
        d->paramInfo.rows          = srcHeight;
        d->paramInfo.cols          = srcWidth;
        d->colorSpace->bitBlt(d->colorSpace, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);
    }
    else {
        /* Read the user selection (d->selection) bytes into an array, ready
        to merge in the next block*/
        quint32 totalBytes = srcWidth * srcHeight * selection->pixelSize();
        quint8 * mergedSelectionBytes = new quint8[ totalBytes ];
        d->selection->projection()->readBytes(mergedSelectionBytes, dstX, dstY, srcWidth, srcHeight);

        // Merge selections here by multiplying them - compositeOp(COMPOSITE_MULT)
        d->paramInfo.dstRowStart   = mergedSelectionBytes;
        d->paramInfo.dstRowStride  = srcWidth * selection->pixelSize();
        d->paramInfo.srcRowStart   = selection->data();
        d->paramInfo.srcRowStride  = srcWidth * selection->pixelSize();
        d->paramInfo.maskRowStart  = 0;
        d->paramInfo.maskRowStride = 0;
        d->paramInfo.rows          = srcHeight;
        d->paramInfo.cols          = srcWidth;
        KoColorSpaceRegistry::instance()->alpha8()->compositeOp(COMPOSITE_MULT)->composite(d->paramInfo);

        // Blit to dstBytes (intermediary bit array)
        d->paramInfo.dstRowStart   = dstBytes;
        d->paramInfo.dstRowStride  = srcWidth * d->device->pixelSize();
        d->paramInfo.srcRowStart   = srcDev->data() + srcX;
        d->paramInfo.srcRowStride  = srcDev->bounds().width() * srcDev->pixelSize();
        d->paramInfo.maskRowStart  = mergedSelectionBytes;
        d->paramInfo.maskRowStride = srcWidth * selection->pixelSize();
        d->colorSpace->bitBlt(d->colorSpace, d->paramInfo, d->compositeOp, d->renderingIntent, d->conversionFlags);

        delete[] mergedSelectionBytes;
    }

    d->device->writeBytes(dstBytes, dstX, dstY, srcWidth, srcHeight);

    delete[] dstBytes;

    addDirtyRect(QRect(dstX, dstY, srcWidth, srcHeight));
}

void KisPainter::bltFixedWithFixedSelection(qint32 dstX, qint32 dstY,
                                            const KisFixedPaintDeviceSP srcDev,
                                            const KisFixedPaintDeviceSP selection,
                                            quint32 srcWidth, quint32 srcHeight)
{
    bltFixedWithFixedSelection(dstX, dstY, srcDev, selection, 0, 0, 0, 0, srcWidth, srcHeight);
}





KisDistanceInformation KisPainter::paintLine(const KisPaintInformation &pi1,
                                             const KisPaintInformation &pi2,
                                             const KisDistanceInformation& savedDist)
{
    if (!d->device) return KisDistanceInformation();
    if (!d->paintOp || !d->paintOp->canPaint()) return KisDistanceInformation();

    return d->paintOp->paintLine(pi1, pi2, savedDist);
}


void KisPainter::paintPolyline(const vQPointF &points,
                               int index, int numPoints)
{
    if (index >= (int) points.count())
        return;

    if (numPoints < 0)
        numPoints = points.count();

    if (index + numPoints > (int) points.count())
        numPoints = points.count() - index;


    KisDistanceInformation saveDist;
    for (int i = index; i < index + numPoints - 1; i++) {
        saveDist = paintLine(points [i], points [i + 1], saveDist);
    }
}

static void getBezierCurvePoints(const KisVector2D &pos1,
                                 const KisVector2D &control1,
                                 const KisVector2D &control2,
                                 const KisVector2D &pos2,
                                 vQPointF& points)
{
    LineEquation line = LineEquation::Through(pos1, pos2);
    qreal d1 = line.absDistance(control1);
    qreal d2 = line.absDistance(control2);

    if (d1 < BEZIER_FLATNESS_THRESHOLD && d2 < BEZIER_FLATNESS_THRESHOLD) {
        points.push_back(toQPointF(pos1));
    } else {
        // Midpoint subdivision. See Foley & Van Dam Computer Graphics P.508

        KisVector2D l2 = (pos1 + control1) / 2;
        KisVector2D h = (control1 + control2) / 2;
        KisVector2D l3 = (l2 + h) / 2;
        KisVector2D r3 = (control2 + pos2) / 2;
        KisVector2D r2 = (h + r3) / 2;
        KisVector2D l4 = (l3 + r2) / 2;

        getBezierCurvePoints(pos1, l2, l3, l4, points);
        getBezierCurvePoints(l4, r2, r3, pos2, points);
    }
}

void KisPainter::getBezierCurvePoints(const QPointF &pos1,
                                      const QPointF &control1,
                                      const QPointF &control2,
                                      const QPointF &pos2,
                                      vQPointF& points) const
{
    ::getBezierCurvePoints(toKisVector2D(pos1), toKisVector2D(control1), toKisVector2D(control2), toKisVector2D(pos2), points);
}

KisDistanceInformation KisPainter::paintBezierCurve(const KisPaintInformation &pi1,
                                                    const QPointF &control1,
                                                    const QPointF &control2,
                                                    const KisPaintInformation &pi2,
                                                    const KisDistanceInformation& savedDist)
{
    if (d->paintOp && d->paintOp->canPaint()) {
        return d->paintOp->paintBezierCurve(pi1, control1, control2, pi2, savedDist);
    }
    return KisDistanceInformation();
}

void KisPainter::paintRect(const QRectF &rect)
{
    QRectF normalizedRect = rect.normalized();

    vQPointF points;

    points.push_back(normalizedRect.topLeft());
    points.push_back(normalizedRect.bottomLeft());
    points.push_back(normalizedRect.bottomRight());
    points.push_back(normalizedRect.topRight());

    paintPolygon(points);
}

void KisPainter::paintRect(const qreal x,
                           const qreal y,
                           const qreal w,
                           const qreal h)
{
    paintRect(QRectF(x, y, w, h));
}

void KisPainter::paintEllipse(const QRectF &rect)
{
    QRectF r = rect.normalized(); // normalize before checking as negative width and height are empty too
    if (r.isEmpty()) return;

    // See http://www.whizkidtech.redprince.net/bezier/circle/ for explanation.
    // kappa = (4/3*(sqrt(2)-1))
    const qreal kappa = 0.5522847498;
    const qreal lx = (r.width() / 2) * kappa;
    const qreal ly = (r.height() / 2) * kappa;

    QPointF center = r.center();

    QPointF p0(r.left(), center.y());
    QPointF p1(r.left(), center.y() - ly);
    QPointF p2(center.x() - lx, r.top());
    QPointF p3(center.x(), r.top());

    vQPointF points;

    getBezierCurvePoints(p0, p1, p2, p3, points);

    QPointF p4(center.x() + lx, r.top());
    QPointF p5(r.right(), center.y() - ly);
    QPointF p6(r.right(), center.y());

    getBezierCurvePoints(p3, p4, p5, p6, points);

    QPointF p7(r.right(), center.y() + ly);
    QPointF p8(center.x() + lx, r.bottom());
    QPointF p9(center.x(), r.bottom());

    getBezierCurvePoints(p6, p7, p8, p9, points);

    QPointF p10(center.x() - lx, r.bottom());
    QPointF p11(r.left(), center.y() + ly);

    getBezierCurvePoints(p9, p10, p11, p0, points);

    paintPolygon(points);
}

void KisPainter::paintEllipse(const qreal x,
                              const qreal y,
                              const qreal w,
                              const qreal h)
{
    paintEllipse(QRectF(x, y, w, h));
}

qreal KisPainter::paintAt(const KisPaintInformation& pi)
{
    if (!d->paintOp || !d->paintOp->canPaint()) return 0.0;
    return d->paintOp->paintAt(pi);
}

void KisPainter::fillPolygon(const vQPointF& points, FillStyle fillStyle)
{
    if (points.count() < 3) {
        return;
    }

    if (fillStyle == FillStyleNone) {
        return;
    }

    QPainterPath polygonPath;

    polygonPath.moveTo(points.at(0));

    for (int pointIndex = 1; pointIndex < points.count(); pointIndex++) {
        polygonPath.lineTo(points.at(pointIndex));
    }

    polygonPath.closeSubpath();

    d->fillStyle = fillStyle;
    fillPainterPath(polygonPath);
}

void KisPainter::paintPolygon(const vQPointF& points)
{
    if (d->fillStyle != FillStyleNone) {
        fillPolygon(points, d->fillStyle);
    }

    if (d->strokeStyle != StrokeStyleNone) {
        if (points.count() > 1) {
            KisDistanceInformation distance;

            for (int i = 0; i < points.count() - 1; i++) {
                distance = paintLine(KisPaintInformation(points[i]), KisPaintInformation(points[i + 1]), distance);
            }
            paintLine(points[points.count() - 1], points[0], distance);
        }
    }
}

void KisPainter::paintPainterPath(const QPainterPath& path)
{
    if (d->fillStyle != FillStyleNone) {
        fillPainterPath(path);
    }

    QPointF lastPoint, nextPoint;
    int elementCount = path.elementCount();
    KisDistanceInformation saveDist;
    for (int i = 0; i < elementCount; i++) {
        QPainterPath::Element element = path.elementAt(i);
        switch (element.type) {
        case QPainterPath::MoveToElement:
            lastPoint =  QPointF(element.x, element.y);
            break;
        case QPainterPath::LineToElement:
            nextPoint =  QPointF(element.x, element.y);
            saveDist = paintLine(KisPaintInformation(lastPoint), KisPaintInformation(nextPoint), saveDist);
            lastPoint = nextPoint;
            break;
        case QPainterPath::CurveToElement:
            nextPoint =  QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y);
            saveDist = paintBezierCurve(KisPaintInformation(lastPoint),
                             QPointF(path.elementAt(i).x, path.elementAt(i).y),
                             QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y),
                             KisPaintInformation(nextPoint), saveDist);
            lastPoint = nextPoint;
            break;
        default:
            continue;
        }
    }
}

void KisPainter::fillPainterPath(const QPainterPath& path)
{
    FillStyle fillStyle = d->fillStyle;

    if (fillStyle == FillStyleNone) {
        return;
    }

    // Fill the polygon bounding rectangle with the required contents then we'll
    // create a mask for the actual polygon coverage.

    if (!d->fillPainter) {
        d->polygon = new KisPaintDevice(d->device->colorSpace());
        d->fillPainter = new KisFillPainter(d->polygon);
    } else {
        d->polygon->clear();
    }

    Q_CHECK_PTR(d->polygon);

    QRectF boundingRect = path.boundingRect();
    QRect fillRect;

    fillRect.setLeft((qint32)floor(boundingRect.left()));
    fillRect.setRight((qint32)ceil(boundingRect.right()));
    fillRect.setTop((qint32)floor(boundingRect.top()));
    fillRect.setBottom((qint32)ceil(boundingRect.bottom()));

    // Expand the rectangle to allow for anti-aliasing.
    fillRect.adjust(-1, -1, 1, 1);

    // Clip to the image bounds.
    if (d->bounds.isValid()) {
        fillRect &= d->bounds;
    }

    switch (fillStyle) {
    default:
        // Fall through
    case FillStyleGradient:
        // Currently unsupported, fall through
    case FillStyleStrokes:
        // Currently unsupported, fall through
        warnImage << "Unknown or unsupported fill style in fillPolygon\n";
    case FillStyleForegroundColor:
        d->fillPainter->fillRect(fillRect, paintColor(), OPACITY_OPAQUE_U8);
        break;
    case FillStyleBackgroundColor:
        d->fillPainter->fillRect(fillRect, backgroundColor(), OPACITY_OPAQUE_U8);
        break;
    case FillStylePattern:
        Q_ASSERT(d->pattern != 0);
        if (d->pattern) { // if the user hasn't got any patterns installed, we shouldn't crash...
            d->fillPainter->fillRect(fillRect, d->pattern);
        }
        break;
    case FillStyleGenerator:
        Q_ASSERT(d->generator != 0);
        if (d->generator) { // if the user hasn't got any generators, we shouldn't crash...
            d->fillPainter->fillRect(fillRect.x(), fillRect.y(), fillRect.width(), fillRect.height(), generator());
        }
        break;
    }

    if (d->polygonMaskImage.isNull() || (d->maskPainter == 0)) {
        d->polygonMaskImage = QImage(d->maskImageWidth, d->maskImageHeight, QImage::Format_ARGB32_Premultiplied);
        d->maskPainter = new QPainter(&d->polygonMaskImage);
        d->maskPainter->setRenderHint(QPainter::Antialiasing, antiAliasPolygonFill());
    }

    // Break the mask up into chunks so we don't have to allocate a potentially very large QImage.
    const QColor black(Qt::black);
    const QBrush brush(Qt::white);
    for (qint32 x = fillRect.x(); x < fillRect.x() + fillRect.width(); x += d->maskImageWidth) {
        for (qint32 y = fillRect.y(); y < fillRect.y() + fillRect.height(); y += d->maskImageHeight) {

            d->polygonMaskImage.fill(black.rgb());
            d->maskPainter->translate(-x, -y);
            d->maskPainter->fillPath(path, brush);
            d->maskPainter->translate(x, y);

            qint32 rectWidth = qMin(fillRect.x() + fillRect.width() - x, d->maskImageWidth);
            qint32 rectHeight = qMin(fillRect.y() + fillRect.height() - y, d->maskImageHeight);

            KisHLineIteratorSP lineIt = d->polygon->createHLineIteratorNG(x, y, rectWidth);

            quint8 tmp;
            for (int row = y; row < y + rectHeight; row++) {
                QRgb* line = reinterpret_cast<QRgb*>(d->polygonMaskImage.scanLine(row - y));
                do {
                    tmp = qRed(line[lineIt->x() - x]);
                    d->polygon->colorSpace()->applyAlphaU8Mask(lineIt->rawData(), &tmp, 1);
                } while (lineIt->nextPixel());
                lineIt->nextRow();
            }

        }
    }

    QRect r = d->polygon->extent();

    // The strokes for the outline may have already added updated the dirtyrect, but it can't hurt,
    // and if we're painting without outlines, then there will be no dirty rect. Let's do it ourselves...
    bitBlt(r.x(), r.y(), d->polygon, r.x(), r.y(), r.width(), r.height());
}

void KisPainter::drawPainterPath(const QPainterPath& path, const QPen& pen)
{
    // we are drawing mask, it has to be white
    // color of the path is given by paintColor()
    Q_ASSERT(pen.color() == Qt::white);

    if (!d->fillPainter) {
        d->polygon = new KisPaintDevice(d->device->colorSpace());
        d->fillPainter = new KisFillPainter(d->polygon);
    } else {
        d->polygon->clear();
    }

    Q_CHECK_PTR(d->polygon);

    QRectF boundingRect = path.boundingRect();
    QRect fillRect;

    fillRect.setLeft((qint32)floor(boundingRect.left()));
    fillRect.setRight((qint32)ceil(boundingRect.right()));
    fillRect.setTop((qint32)floor(boundingRect.top()));
    fillRect.setBottom((qint32)ceil(boundingRect.bottom()));

    // take width of the pen into account
    int penWidth = qRound(pen.widthF());
    fillRect.adjust(-penWidth, -penWidth, penWidth, penWidth);

    // Expand the rectangle to allow for anti-aliasing.
    fillRect.adjust(-1, -1, 1, 1);


    // Clip to the image bounds.
    if (d->bounds.isValid()) {
        fillRect &= d->bounds;
    }

    d->fillPainter->fillRect(fillRect, paintColor(), OPACITY_OPAQUE_U8);

    if (d->polygonMaskImage.isNull() || (d->maskPainter == 0)) {
        d->polygonMaskImage = QImage(d->maskImageWidth, d->maskImageHeight, QImage::Format_ARGB32_Premultiplied);
        d->maskPainter = new QPainter(&d->polygonMaskImage);
        d->maskPainter->setRenderHint(QPainter::Antialiasing, antiAliasPolygonFill());
    }

    // Break the mask up into chunks so we don't have to allocate a potentially very large QImage.
    const QColor black(Qt::black);
    QPen oldPen = d->maskPainter->pen();
    d->maskPainter->setPen(pen);

    for (qint32 x = fillRect.x(); x < fillRect.x() + fillRect.width(); x += d->maskImageWidth) {
        for (qint32 y = fillRect.y(); y < fillRect.y() + fillRect.height(); y += d->maskImageHeight) {

            d->polygonMaskImage.fill(black.rgb());
            d->maskPainter->translate(-x, -y);
            d->maskPainter->drawPath(path);
            d->maskPainter->translate(x, y);

            qint32 rectWidth = qMin(fillRect.x() + fillRect.width() - x, d->maskImageWidth);
            qint32 rectHeight = qMin(fillRect.y() + fillRect.height() - y, d->maskImageHeight);

            KisHLineIteratorSP lineIt = d->polygon->createHLineIteratorNG(x, y, rectWidth);

            quint8 tmp;
            for (int row = y; row < y + rectHeight; row++) {
                QRgb* line = reinterpret_cast<QRgb*>(d->polygonMaskImage.scanLine(row - y));
                do {
                    tmp = qRed(line[lineIt->x() - x]);
                    d->polygon->colorSpace()->applyAlphaU8Mask(lineIt->rawData(), &tmp, 1);
                } while (lineIt->nextPixel());
                lineIt->nextRow();
            }

        }
    }

    d->maskPainter->setPen(oldPen);
    QRect r = d->polygon->extent();

    bitBlt(r.x(), r.y(), d->polygon, r.x(), r.y(), r.width(), r.height());
}

/**/
void KisPainter::drawLine(const QPointF& start, const QPointF& end, qreal width, bool antialias){
    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();

    if ((x2 == x1 ) && (y2 == y1)) return;

    int dstX = x2-x1;
    int dstY = y2-y1;

    qreal _C = dstX*y1 - dstY*x1;
    qreal projectionDenominator = 1.0 / (pow((double)dstX, 2) + pow((double)dstY, 2));

    qreal subPixel;
    if (qAbs(dstX) > qAbs(dstY)){
        subPixel = start.x() - x1;
    }else{
        subPixel = start.y() - y1;
    }

    qreal halfWidth = width * 0.5 + subPixel;
    int W_ = qRound(halfWidth) + 1;

    // save the state
    int X1_ = x1;
    int Y1_ = y1;
    int X2_ = x2;
    int Y2_ = y2;

    if (x2<x1) qSwap(x1,x2);
    if (y2<y1) qSwap(y1,y2);

    qreal denominator = sqrt(pow((double)dstY,2) + pow((double)dstX,2));
    if (denominator == 0.0) {
        denominator = 1.0;
    }
    denominator = 1.0/denominator;

    qreal projection,scanX,scanY,AA_;
    quint8 pixelOpacity = quint8(d->paramInfo.opacity * 255.0f);
    int pixelSize = d->device->pixelSize();
    KisRandomAccessorSP accessor = d->device->createRandomAccessorNG(x1, y1);
    KisRandomConstAccessorSP selectionAccessor;
    if (d->selection) {
        selectionAccessor = d->selection->projection()->createRandomConstAccessorNG(x1, y1);
    }

    for (int y = y1-W_; y < y2+W_ ; y++){
        for (int x = x1-W_; x < x2+W_; x++){

            projection = ( (x-X1_)* dstX + (y-Y1_)*dstY ) * projectionDenominator;
            scanX = X1_ + projection * dstX;
            scanY = Y1_ + projection * dstY;

            if (((scanX < x1) || (scanX > x2)) || ((scanY < y1) || (scanY > y2))) {
                AA_ = qMin( sqrt( pow((double)x - X1_, 2) + pow((double)y - Y1_, 2) ),
                            sqrt( pow((double)x - X2_, 2) + pow((double)y - Y2_, 2) ));
            }else{
                AA_ = qAbs(dstY*x - dstX*y + _C) * denominator;
            }

            if (AA_>halfWidth) {
                continue;
            }

            if (antialias) {
                if (AA_ > halfWidth-1.0) {
                    pixelOpacity = quint8((d->paramInfo.opacity*255.0f) * ( 1.0 - (AA_-(halfWidth-1.0))));
                }else{
                    pixelOpacity = quint8(d->paramInfo.opacity*255.0f);
                }
            }

            accessor->moveTo(x, y);
            if (selectionAccessor) selectionAccessor->moveTo(x,y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, d->paintColor.data() , pixelSize, 0,0,1,1,pixelOpacity);
            }
        }
    }
}

/**/

void KisPainter::drawLine(const QPointF & start, const QPointF & end)
{
    drawThickLine(start, end, 1, 1);
}


void KisPainter::drawDDALine(const QPointF & start, const QPointF & end)
{
    int x = int(start.x());
    int y = int(start.y());

    int x2 = int(end.x());
    int y2 = int(end.y());

    // Width and height of the line
    int xd = x2 - x;
    int yd = y2 - y;

    float m = (float)yd / (float)xd;

    float fx = x;
    float fy = y;
    int inc;

    int    pixelSize    = d->device->pixelSize();
    quint8 pixelOpacity = quint8(d->paramInfo.opacity*255.0f);

    KisRandomAccessorSP accessor = d->device->createRandomAccessorNG(x, y);
    KisRandomConstAccessorSP selectionAccessor;
    if (d->selection) {
        selectionAccessor = d->selection->projection()->createRandomConstAccessorNG(x, y);
    }


    accessor->moveTo(x, y);
    if (selectionAccessor) selectionAccessor->moveTo(x,y);

    if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
        d->compositeOp->composite(accessor->rawData(), pixelSize, d->paintColor.data(), pixelSize, 0,0,1,1, pixelOpacity);
    }

    if (fabs(m) > 1.0f) {
        inc = (yd > 0) ? 1 : -1;
        m = 1.0f / m;
        m *= inc;
        while (y != y2) {
            y = y + inc;
            fx = fx + m;
            x = qRound(fx);

            accessor->moveTo(x, y);
            if (selectionAccessor) selectionAccessor->moveTo(x, y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, d->paintColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
    } else {
        inc = (xd > 0) ? 1 : -1;
        m *= inc;
        while (x != x2) {
            x = x + inc;
            fy = fy + m;
            y = qRound(fy);

            accessor->moveTo(x, y);
            if (selectionAccessor) selectionAccessor->moveTo(x, y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, d->paintColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
    }
}

void KisPainter::drawWobblyLine(const QPointF & start, const QPointF & end)
{
    KisRandomAccessorSP accessor = d->device->createRandomAccessorNG(start.x(), start.y());
    KisRandomConstAccessorSP selectionAccessor;
    if (d->selection) {
        selectionAccessor = d->selection->projection()->createRandomConstAccessorNG(start.x(), start.y());
    }

    int     pixelSize    = d->device->pixelSize();
    quint8  pixelOpacity = quint8(d->paramInfo.opacity*255.0f);
    KoColor mycolor(d->paintColor);

    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();

    // Width and height of the line
    int xd = (x2 - x1);
    int yd = (y2 - y1);

    int x;
    int y;
    float fx = (x = x1);
    float fy = (y = y1);
    float m = (float)yd / (float)xd;
    int inc;

    if (fabs(m) > 1) {
        inc = (yd > 0) ? 1 : -1;
        m = 1.0f / m;
        m *= inc;
        while (y != y2) {
            fx = fx + m;
            y = y + inc;
            x = qRound(fx);

            float br1 = int(fx + 1) - fx;
            float br2 = fx - (int)fx;

            accessor->moveTo(x, y);
            if (selectionAccessor) selectionAccessor->moveTo(x, y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                mycolor.setOpacity((quint8)(255*br1));
                d->compositeOp->composite(accessor->rawData(), pixelSize, mycolor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            accessor->moveTo(x + 1, y);
            if (selectionAccessor) selectionAccessor->moveTo(x + 1, y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                mycolor.setOpacity((quint8)(255*br2));
                d->compositeOp->composite(accessor->rawData(), pixelSize, mycolor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
    } else {
        inc = (xd > 0) ? 1 : -1;
        m *= inc;
        while (x != x2) {
            fy = fy + m;
            x = x + inc;
            y = qRound(fy);

            float br1 = int(fy + 1) - fy;
            float br2 = fy - (int)fy;

            accessor->moveTo(x, y);
            if (selectionAccessor) selectionAccessor->moveTo(x, y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                mycolor.setOpacity((quint8)(255*br1));
                d->compositeOp->composite(accessor->rawData(), pixelSize, mycolor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            accessor->moveTo(x, y + 1);
            if (selectionAccessor) selectionAccessor->moveTo(x, y + 1);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                mycolor.setOpacity((quint8)(255*br2));
                d->compositeOp->composite(accessor->rawData(), pixelSize, mycolor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
    }

}

void KisPainter::drawWuLine(const QPointF & start, const QPointF & end)
{
    KisRandomAccessorSP accessor = d->device->createRandomAccessorNG(start.x(), start.y());
    KisRandomConstAccessorSP selectionAccessor;
    if (d->selection) {
        selectionAccessor = d->selection->projection()->createRandomConstAccessorNG(start.x(), start.y());
    }

    int     pixelSize    = d->device->pixelSize();
    quint8  pixelOpacity = quint8(d->paramInfo.opacity*255.0f);
    KoColor lineColor(d->paintColor);

    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();


    float grad, xd, yd;
    float xgap, ygap, xend, yend, yf, xf;
    float brightness1, brightness2;

    int ix1, ix2, iy1, iy2;
    quint8 c1, c2;

    // gradient of line
    xd = (x2 - x1);
    yd = (y2 - y1);

    if (yd == 0) {
        /* Horizontal line */
        int incr = (x1 < x2) ? 1 : -1;
        ix1 = (int)x1;
        ix2 = (int)x2;
        iy1 = (int)y1;
        while (ix1 != ix2) {
            ix1 = ix1 + incr;

            accessor->moveTo(ix1, iy1);
            if (selectionAccessor) selectionAccessor->moveTo(ix1, iy1);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
        return;
    }

    if (xd == 0) {
        /* Vertical line */
        int incr = (y1 < y2) ? 1 : -1;
        iy1 = (int)y1;
        iy2 = (int)y2;
        ix1 = (int)x1;
        while (iy1 != iy2) {
            iy1 = iy1 + incr;

            accessor->moveTo(ix1, iy1);
            if (selectionAccessor) selectionAccessor->moveTo(ix1, iy1);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
        return;
    }

    if (fabs(xd) > fabs(yd)) {
        // horizontal line
        // line have to be paint from left to right
        if (x1 > x2) {
            float tmp;
            tmp = x1; x1 = x2; x2 = tmp;
            tmp = y1; y1 = y2; y2 = tmp;
            xd = (x2 - x1);
            yd = (y2 - y1);
        }
        grad = yd / xd;
        // nearest X,Y interger coordinates
        xend = static_cast<int>(x1 + 0.5f);
        yend = y1 + grad * (xend - x1);

        xgap = invertFrac(x1 + 0.5f);

        ix1 = static_cast<int>(xend);
        iy1 = static_cast<int>(yend);

        // calc the intensity of the other end point pixel pair.
        brightness1 = invertFrac(yend) * xgap;
        brightness2 =       frac(yend) * xgap;

        c1 = (int)(brightness1 * OPACITY_OPAQUE_U8);
        c2 = (int)(brightness2 * OPACITY_OPAQUE_U8);

        accessor->moveTo(ix1, iy1);
        if (selectionAccessor) selectionAccessor->moveTo(ix1, iy1);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c1);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        accessor->moveTo(ix1, iy1 + 1);
        if (selectionAccessor) selectionAccessor->moveTo(ix1, iy1 + 1);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c2);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        // calc first Y-intersection for main loop
        yf = yend + grad;

        xend = trunc(x2 + 0.5f);
        yend = y2 + grad * (xend - x2);

        xgap = invertFrac(x2 - 0.5f);

        ix2 = static_cast<int>(xend);
        iy2 = static_cast<int>(yend);

        brightness1 = invertFrac(yend) * xgap;
        brightness2 =    frac(yend) * xgap;

        c1 = (int)(brightness1 * OPACITY_OPAQUE_U8);
        c2 = (int)(brightness2 * OPACITY_OPAQUE_U8);

        accessor->moveTo(ix2, iy2);
        if (selectionAccessor) selectionAccessor->moveTo(ix2, iy2);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c1);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        accessor->moveTo(ix2, iy2 + 1);
        if (selectionAccessor) selectionAccessor->moveTo(ix2, iy2 + 1);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c2);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        // main loop
        for (int x = ix1 + 1; x <= ix2 - 1; x++) {
            brightness1 = invertFrac(yf);
            brightness2 =    frac(yf);
            c1 = (int)(brightness1 * OPACITY_OPAQUE_U8);
            c2 = (int)(brightness2 * OPACITY_OPAQUE_U8);

            accessor->moveTo(x, int (yf));
            if (selectionAccessor) selectionAccessor->moveTo(x, int (yf));

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                lineColor.setOpacity(c1);
                d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            accessor->moveTo(x, int (yf) + 1);
            if (selectionAccessor) selectionAccessor->moveTo(x, int (yf) + 1);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                lineColor.setOpacity(c2);
                d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            yf = yf + grad;
        }
    } else {
        //vertical
        // line have to be painted from left to right
        if (y1 > y2) {
            float tmp;
            tmp = x1; x1 = x2; x2 = tmp;
            tmp = y1; y1 = y2; y2 = tmp;
            xd = (x2 - x1);
            yd = (y2 - y1);
        }

        grad = xd / yd;

        // nearest X,Y interger coordinates
        yend = static_cast<int>(y1 + 0.5f);
        xend = x1 + grad * (yend - y1);

        ygap = invertFrac(y1 + 0.5f);

        ix1 = static_cast<int>(xend);
        iy1 = static_cast<int>(yend);

        // calc the intensity of the other end point pixel pair.
        brightness1 = invertFrac(xend) * ygap;
        brightness2 =       frac(xend) * ygap;

        c1 = (int)(brightness1 * OPACITY_OPAQUE_U8);
        c2 = (int)(brightness2 * OPACITY_OPAQUE_U8);

        accessor->moveTo(ix1, iy1);
        if (selectionAccessor) selectionAccessor->moveTo(ix1, iy1);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c1);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        accessor->moveTo(x1 + 1, y1);
        if (selectionAccessor) selectionAccessor->moveTo(x1 + 1, y1);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c2);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        // calc first Y-intersection for main loop
        xf = xend + grad;

        yend = trunc(y2 + 0.5f);
        xend = x2 + grad * (yend - y2);

        ygap = invertFrac(y2 - 0.5f);

        ix2 = static_cast<int>(xend);
        iy2 = static_cast<int>(yend);

        brightness1 = invertFrac(xend) * ygap;
        brightness2 =    frac(xend) * ygap;

        c1 = (int)(brightness1 * OPACITY_OPAQUE_U8);
        c2 = (int)(brightness2 * OPACITY_OPAQUE_U8);

        accessor->moveTo(ix2, iy2);
        if (selectionAccessor) selectionAccessor->moveTo(ix2, iy2);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c1);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        accessor->moveTo(ix2 + 1, iy2);
        if (selectionAccessor) selectionAccessor->moveTo(ix2 + 1, iy2);

        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
            lineColor.setOpacity(c2);
            d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
        }

        // main loop
        for (int y = iy1 + 1; y <= iy2 - 1; y++) {
            brightness1 = invertFrac(xf);
            brightness2 =    frac(xf);
            c1 = (int)(brightness1 * OPACITY_OPAQUE_U8);
            c2 = (int)(brightness2 * OPACITY_OPAQUE_U8);

            accessor->moveTo(int (xf), y);
            if (selectionAccessor) selectionAccessor->moveTo(int (xf), y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                lineColor.setOpacity(c1);
                d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            accessor->moveTo(int (xf) + 1, y);
            if (selectionAccessor) selectionAccessor->moveTo(int (xf) + 1, y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                lineColor.setOpacity(c2);
                d->compositeOp->composite(accessor->rawData(), pixelSize, lineColor.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            xf = xf + grad;
        }
    }//end-of-else

}

void KisPainter::drawThickLine(const QPointF & start, const QPointF & end, int startWidth, int endWidth)
{

    KisRandomAccessorSP accessor = d->device->createRandomAccessorNG(start.x(), start.y());
    KisRandomConstAccessorSP selectionAccessor;
    if (d->selection) {
        selectionAccessor = d->selection->projection()->createRandomConstAccessorNG(start.x(), start.y());
    }

    int           pixelSize    = d->device->pixelSize();
    quint8        pixelOpacity = quint8(d->paramInfo.opacity*255.0f);
    KoColorSpace* cs           = d->device->colorSpace();

    KoColor c1(d->paintColor);
    KoColor c2(d->paintColor);
    KoColor c3(d->paintColor);
    KoColor col1(c1);
    KoColor col2(c1);

    float grada, gradb, dxa, dxb, dya, dyb, fraca, fracb,
    xfa, yfa, xfb, yfb, b1a, b2a, b1b, b2b, dstX, dstY;
    int x, y, ix1, ix2, iy1, iy2;

    int x0a, y0a, x1a, y1a, x0b, y0b, x1b, y1b;
    int tp0, tn0, tp1, tn1;

    int horizontal = 0;
    float opacity = 1.0;

    tp0 = startWidth / 2;
    tn0 = startWidth / 2;
    if (startWidth % 2 == 0) // even width startWidth
        tn0--;

    tp1 = endWidth / 2;
    tn1 = endWidth / 2;
    if (endWidth % 2 == 0) // even width endWidth
        tn1--;

    int x0 = qRound(start.x());
    int y0 = qRound(start.y());
    int x1 = qRound(end.x());
    int y1 = qRound(end.y());

    dstX = x1 - x0; // run of general line
    dstY = y1 - y0; // rise of general line

    if (dstY < 0) dstY = -dstY;
    if (dstX < 0) dstX = -dstX;

    if (dstX > dstY) { // horizontalish
        horizontal = 1;
        x0a = x0;   y0a = y0 - tn0;
        x0b = x0;   y0b = y0 + tp0;
        x1a = x1;   y1a = y1 - tn1;
        x1b = x1;   y1b = y1 + tp1;
    } else {
        x0a = x0 - tn0;   y0a = y0;
        x0b = x0 + tp0;   y0b = y0;
        x1a = x1 - tn1;   y1a = y1;
        x1b = x1 + tp1;   y1b = y1;
    }

    if (horizontal) { // draw endpoints
        for (int i = y0a; i <= y0b; i++) {

            accessor->moveTo(x0, i);
            if (selectionAccessor) selectionAccessor->moveTo(x0, i);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, c1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
        for (int i = y1a; i <= y1b; i++) {

            accessor->moveTo(x1, i);
            if (selectionAccessor) selectionAccessor->moveTo(x1, i);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, c1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }

    } else {
        for (int i = x0a; i <= x0b; i++) {

            accessor->moveTo(i, y0);
            if (selectionAccessor) selectionAccessor->moveTo(i, y0);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, c1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
        for (int i = x1a; i <= x1b; i++) {
            accessor->moveTo(i, y1);
            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                d->compositeOp->composite(accessor->rawData(), pixelSize, c1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
    }

    //antialias endpoints
    if (x1 != x0 && y1 != y0) {
        if (horizontal) {

            accessor->moveTo(x0a, y0a - 1);
            if (selectionAccessor) selectionAccessor->moveTo(x0a, y0a - 1);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                qreal alpha = cs->opacityF(accessor->rawData());
                opacity = .25 * c1.opacityF() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            accessor->moveTo(x1b, y1b + 1);
            if (selectionAccessor) selectionAccessor->moveTo(x1b, y1b + 1);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                qreal alpha = cs->opacityF(accessor->rawData());
                opacity = .25 * c2.opacityF() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

        } else {

            accessor->moveTo(x0a - 1, y0a);
            if (selectionAccessor) selectionAccessor->moveTo(x0a - 1, y0a);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                qreal alpha = cs->opacityF(accessor->rawData());
                opacity = .25 * c1.opacityF() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            accessor->moveTo(x1b + 1, y1b);
            if (selectionAccessor) selectionAccessor->moveTo(x1b + 1, y1b);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                qreal alpha = cs->opacityF(accessor->rawData());
                opacity = .25 * c2.opacityF() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }
        }
    }

    dxa = x1a - x0a; // run of a
    dya = y1a - y0a; // rise of a
    dxb = x1b - x0b; // run of b
    dyb = y1b - y0b; // rise of b

    if (horizontal) { // horizontal-ish lines
        if (x1 < x0) {
            int xt, yt, wt;
            KoColor tmp;
            xt = x1a;     x1a = x0a;    x0a = xt;
            yt = y1a;     y1a = y0a;    y0a = yt;
            xt = x1b;     x1b = x0b;    x0b = xt;
            yt = y1b;     y1b = y0b;    y0b = yt;
            xt = x1;      x1 = x0;      x0 = xt;
            yt = y1;      y1 = y0;      y0 = yt;

            tmp = c1; c1 = c2; c2 = tmp;
            wt = startWidth;      startWidth = endWidth;      endWidth = wt;
        }

        grada = dya / dxa;
        gradb = dyb / dxb;

        ix1 = x0;   iy1 = y0;
        ix2 = x1;   iy2 = y1;

        yfa = y0a + grada;
        yfb = y0b + gradb;

        for (x = ix1 + 1; x <= ix2 - 1; x++) {
            fraca = yfa - int (yfa);
            b1a = 1 - fraca;
            b2a = fraca;

            fracb = yfb - int (yfb);
            b1b = 1 - fracb;
            b2b = fracb;

            // color first pixel of bottom line
            opacity = ((x - ix1) / dstX) * c2.opacityF() + (1 - (x - ix1) / dstX) * c1.opacityF();
            c3.setOpacity(opacity);

            accessor->moveTo(x, (int)yfa);
            if (selectionAccessor) selectionAccessor->moveTo(x, (int)yfa);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                qreal alpha = cs->opacityF(accessor->rawData());
                opacity = b1a * c3.opacityF() + (1 - b1a) * alpha;
                col1.setOpacity(opacity);
                d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            // color first pixel of top line
            if (!(startWidth == 1 && endWidth == 1)) {
                accessor->moveTo(x, (int)yfb);
                if (selectionAccessor) selectionAccessor->moveTo(x, (int)yfb);

                if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                    qreal alpha = cs->opacityF(accessor->rawData());
                    opacity = b1b * c3.opacityF() + (1 - b1b) * alpha;
                    col1.setOpacity(opacity);
                    d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
                }
            }

            // color second pixel of bottom line
            if (grada != 0 && grada != 1) { // if not flat or exact diagonal

                accessor->moveTo(x, int (yfa) + 1);
                if (selectionAccessor) selectionAccessor->moveTo(x, int (yfa) + 1);

                if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                    qreal alpha = cs->opacityF(accessor->rawData());
                    opacity = b2a * c3.opacityF() + (1 - b2a)  * alpha;
                    col2.setOpacity(opacity);
                    d->compositeOp->composite(accessor->rawData(), pixelSize, col2.data() , pixelSize, 0,0,1,1, pixelOpacity);
                }

            }

            // color second pixel of top line
            if (gradb != 0 && gradb != 1 && !(startWidth == 1 && endWidth == 1)) {

                accessor->moveTo(x, int (yfb) + 1);
                if (selectionAccessor) selectionAccessor->moveTo(x, int (yfb) + 1);

                if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                    qreal alpha = cs->opacityF(accessor->rawData());
                    opacity = b2b * c3.opacityF() + (1 - b2b) * alpha;
                    col2.setOpacity(opacity);
                    d->compositeOp->composite(accessor->rawData(), pixelSize, col2.data() , pixelSize, 0,0,1,1, pixelOpacity);
                }

            }

            // fill remaining pixels
            if (!(startWidth == 1 && endWidth == 1)) {
                if (yfa < yfb)
                    for (int i = yfa + 1; i <= yfb; i++) {

                        accessor->moveTo(x, i);
                        if (selectionAccessor) selectionAccessor->moveTo(x, i);

                        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                            d->compositeOp->composite(accessor->rawData(), pixelSize, c3.data() , pixelSize, 0,0,1,1, pixelOpacity);
                        }
                    }
                else
                    for (int i = yfa + 1; i >= yfb; i--) {

                        accessor->moveTo(x, i);
                        if (selectionAccessor) selectionAccessor->moveTo(x, i);

                        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                            d->compositeOp->composite(accessor->rawData(), pixelSize, c3.data() , pixelSize, 0,0,1,1, pixelOpacity);
                        }
                    }

            }

            yfa += grada;
            yfb += gradb;
        }
    } else { // vertical-ish lines
        if (y1 < y0) {
            int xt, yt, wt;
            xt = x1a;     x1a = x0a;    x0a = xt;
            yt = y1a;     y1a = y0a;    y0a = yt;
            xt = x1b;     x1b = x0b;    x0b = xt;
            yt = y1b;     y1b = y0b;    y0b = yt;
            xt = x1;      x1 = x0;      x0 = xt;
            yt = y1;      y1 = y0;      y0 = yt;

            KoColor tmp;
            tmp = c1; c1 = c2; c2 = tmp;
            wt = startWidth;      startWidth = endWidth;      endWidth = wt;
        }

        grada = dxa / dya;
        gradb = dxb / dyb;

        ix1 = x0;   iy1 = y0;
        ix2 = x1;   iy2 = y1;

        xfa = x0a + grada;
        xfb = x0b + gradb;

        for (y = iy1 + 1; y <= iy2 - 1; y++) {
            fraca = xfa - int (xfa);
            b1a = 1 - fraca;
            b2a = fraca;

            fracb = xfb - int (xfb);
            b1b = 1 - fracb;
            b2b = fracb;

            // color first pixel of left line
            opacity = ((y - iy1) / dstY) * c2.opacityF() + (1 - (y - iy1) / dstY) * c1.opacityF();
            c3.setOpacity(opacity);

            accessor->moveTo(int (xfa), y);
            if (selectionAccessor) selectionAccessor->moveTo(int (xfa), y);

            if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                qreal alpha = cs->opacityF(accessor->rawData());
                opacity = b1a * c3.opacityF() + (1 - b1a) * alpha;
                col1.setOpacity(opacity);
                d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
            }

            // color first pixel of right line
            if (!(startWidth == 1 && endWidth == 1)) {

                accessor->moveTo(int(xfb), y);
                if (selectionAccessor) selectionAccessor->moveTo(int(xfb), y);

                if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                    qreal alpha = cs->opacityF(accessor->rawData());
                    opacity = b1b * c3.opacityF() + (1 - b1b)  * alpha;
                    col1.setOpacity(opacity);
                    d->compositeOp->composite(accessor->rawData(), pixelSize, col1.data() , pixelSize, 0,0,1,1, pixelOpacity);
                }
            }

            // color second pixel of left line
            if (grada != 0 && grada != 1) { // if not flat or exact diagonal

                accessor->moveTo(int(xfa) + 1, y);
                if (selectionAccessor) selectionAccessor->moveTo(int(xfa) + 1, y);

                if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                    qreal alpha = cs->opacityF(accessor->rawData());
                    opacity = b2a * c3.opacityF() + (1 - b2a) * alpha;
                    col2.setOpacity(opacity);
                    d->compositeOp->composite(accessor->rawData(), pixelSize, col2.data() , pixelSize, 0,0,1,1, pixelOpacity);
                }

            }

            // color second pixel of right line
            if (gradb != 0 && gradb != 1 && !(startWidth == 1 && endWidth == 1)) {

                accessor->moveTo(int(xfb) + 1, y);
                if (selectionAccessor) selectionAccessor->moveTo(int(xfb) + 1, y);

                if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                    qreal alpha = cs->opacityF(accessor->rawData());
                    opacity = b2b * c3.opacityF() + (1 - b2b) * alpha;
                    col2.setOpacity(opacity);
                    d->compositeOp->composite(accessor->rawData(), pixelSize, col2.data() , pixelSize, 0,0,1,1, pixelOpacity);
                }
            }

            // fill remaining pixels between current xfa,xfb
            if (!(startWidth == 1 && endWidth == 1)) {
                if (xfa < xfb)
                    for (int i = (int) xfa + 1; i <= (int) xfb; i++) {

                        accessor->moveTo(i, y);
                        if (selectionAccessor) selectionAccessor->moveTo(i, y);

                        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                            d->compositeOp->composite(accessor->rawData(), pixelSize, c3.data() , pixelSize, 0,0,1,1, pixelOpacity);
                        }
                    }
                else
                    for (int i = (int) xfb; i <= (int) xfa + 1; i++) {

                        accessor->moveTo(i, y);
                        if (selectionAccessor) selectionAccessor->moveTo(i, y);

                        if (!selectionAccessor || *selectionAccessor->oldRawData() > SELECTION_THRESHOLD) {
                            d->compositeOp->composite(accessor->rawData(), pixelSize, c3.data() , pixelSize, 0,0,1,1, pixelOpacity);
                        }
                    }
            }

            xfa += grada;
            xfb += gradb;
        }
    }

}



void KisPainter::setProgress(KoUpdater * progressUpdater)
{
    d->progressUpdater = progressUpdater;
}

const KisPaintDeviceSP KisPainter::device() const
{
    return d->device;
}
KisPaintDeviceSP KisPainter::device()
{
    return d->device;
}

void KisPainter::setChannelFlags(QBitArray channelFlags)
{
    Q_ASSERT(channelFlags.isEmpty() || quint32(channelFlags.size()) == d->colorSpace->channelCount());
    // Now, if all bits in the channelflags are true, pass an empty channel flags bitarray
    // because otherwise the compositeops cannot optimize.
    d->paramInfo.channelFlags = channelFlags;

    if (!channelFlags.isEmpty() && channelFlags == QBitArray(channelFlags.size(), true)) {
        d->paramInfo.channelFlags = QBitArray();
    }
}

QBitArray KisPainter::channelFlags()
{
    return d->paramInfo.channelFlags;
}

void KisPainter::setPattern(const KisPattern * pattern)
{
    d->pattern = pattern;
}

const KisPattern * KisPainter::pattern() const
{
    return d->pattern;
}

void KisPainter::setPaintColor(const KoColor& color)
{
    d->paintColor = color;
    if (d->device) {
        d->paintColor.convertTo(d->device->colorSpace());
    }
}

const KoColor &KisPainter::paintColor() const
{
    return d->paintColor;
}

void KisPainter::setBackgroundColor(const KoColor& color)
{
    d->backgroundColor = color;
    if (d->device) {
        d->backgroundColor.convertTo(d->device->colorSpace());
    }
}

const KoColor &KisPainter::backgroundColor() const
{
    return d->backgroundColor;
}

void KisPainter::setFillColor(const KoColor& color)
{
    d->fillColor = color;
}

const KoColor &KisPainter::fillColor() const
{
    return d->fillColor;
}

void KisPainter::setGenerator(const KisFilterConfiguration * generator)
{
    d->generator = generator;
}

const KisFilterConfiguration * KisPainter::generator() const
{
    return d->generator;
}

void KisPainter::setFillStyle(FillStyle fillStyle)
{
    d->fillStyle = fillStyle;
}

KisPainter::FillStyle KisPainter::fillStyle() const
{
    return d->fillStyle;
}

void KisPainter::setAntiAliasPolygonFill(bool antiAliasPolygonFill)
{
    d->antiAliasPolygonFill = antiAliasPolygonFill;
}

bool KisPainter::antiAliasPolygonFill()
{
    return d->antiAliasPolygonFill;
}

void KisPainter::setStrokeStyle(KisPainter::StrokeStyle strokeStyle)
{
    d->strokeStyle = strokeStyle;
}
KisPainter::StrokeStyle KisPainter::strokeStyle() const
{
    return d->strokeStyle;
}

void KisPainter::setFlow(quint8 flow)
{
    d->paramInfo.flow = float(flow) / 255.0f;
}

quint8 KisPainter::flow() const
{
    return quint8(d->paramInfo.flow * 255.0f);
}

void KisPainter::setOpacity(quint8 opacity)
{
    d->paramInfo.opacity = float(opacity) / 255.0f;
}

quint8 KisPainter::opacity() const
{
    return quint8(d->paramInfo.opacity * 255.0f);
}

void KisPainter::setBounds(const QRect & bounds)
{
    d->bounds = bounds;
}

QRect KisPainter::bounds()
{
    return d->bounds;
}

void KisPainter::setCompositeOp(const KoCompositeOp * op)
{
    d->compositeOp = op;
}

const KoCompositeOp * KisPainter::compositeOp()
{
    return d->compositeOp;
}

void KisPainter::setCompositeOp(const QString& op)
{
    d->compositeOp = d->colorSpace->compositeOp(op);
}

void KisPainter::setSelection(KisSelectionSP selection)
{
    d->selection = selection;
}

KisSelectionSP KisPainter::selection()
{
    return d->selection;
}

KoUpdater * KisPainter::progressUpdater()
{
    return d->progressUpdater;
}

void KisPainter::setGradient(const KoAbstractGradient* gradient)
{
    d->gradient = gradient;
}

const KoAbstractGradient* KisPainter::gradient() const
{
    return d->gradient;
}

void KisPainter::setPaintOpPreset(KisPaintOpPresetSP preset, KisImageWSP image)
{
    d->paintOpPreset = preset;
    KisPaintOp *paintop = KisPaintOpRegistry::instance()->paintOp(preset, this, image);
    Q_ASSERT(paintop);
    if (paintop) {
        delete d->paintOp;
        d->paintOp = paintop;
    }
    else {
        qWarning() << "Could not create paintop for preset " << preset->name();
    }
}

KisPaintOpPresetSP KisPainter::preset() const
{
    return d->paintOpPreset;
}

KisPaintOp* KisPainter::paintOp() const
{
    return d->paintOp;
}

void KisPainter::setMirrorInformation(const QPointF& axisCenter, bool mirrorHorizontaly, bool mirrorVerticaly)
{
    d->axisCenter = axisCenter;
    d->mirrorHorizontaly = mirrorHorizontaly;
    d->mirrorVerticaly = mirrorVerticaly;
}

void KisPainter::copyMirrorInformation(KisPainter* painter)
{
    painter->setMirrorInformation(d->axisCenter, d->mirrorHorizontaly, d->mirrorVerticaly);
}


void KisPainter::setMaskImageSize(qint32 width, qint32 height)
{

    d->maskImageWidth = qBound(1, width, 256);
    d->maskImageHeight = qBound(1, height, 256);
    d->fillPainter = 0;
    d->polygonMaskImage = QImage();
}

void KisPainter::setLockAlpha(bool protect)
{
    if(d->paramInfo.channelFlags.isEmpty()) {
        d->paramInfo.channelFlags = d->colorSpace->channelFlags(true, true);
    }

    QBitArray switcher =
        d->colorSpace->channelFlags(protect, !protect);

    if(protect) {
        d->paramInfo.channelFlags &= switcher;
    }
    else {
        d->paramInfo.channelFlags |= switcher;
    }

    Q_ASSERT(quint32(d->paramInfo.channelFlags.size()) == d->colorSpace->channelCount());
}

bool KisPainter::alphaLocked() const
{
    QBitArray switcher = d->colorSpace->channelFlags(false, true);
    return !(d->paramInfo.channelFlags & switcher).count(true);
}

void KisPainter::setRenderingIntent(KoColorConversionTransformation::Intent intent)
{
    d->renderingIntent = intent;
}

void KisPainter::setColorConversionFlags(KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    d->conversionFlags = conversionFlags;
}

void KisPainter::renderMirrorMask(QRect rc, KisFixedPaintDeviceSP dab)
{
    int x = rc.topLeft().x();
    int y = rc.topLeft().y();

    int mirrorX = -((x+rc.width()) - d->axisCenter.x()) + d->axisCenter.x();
    int mirrorY = -((y+rc.height()) - d->axisCenter.y()) + d->axisCenter.y();

    if (d->mirrorHorizontaly && d->mirrorVerticaly){
        dab->mirror(true, false);
        bltFixed(mirrorX, y, dab, 0,0,rc.width(),rc.height());
        dab->mirror(false,true);
        bltFixed(mirrorX, mirrorY, dab, 0,0,rc.width(),rc.height());
        dab->mirror(true, false);
        bltFixed(x, mirrorY, dab, 0,0,rc.width(),rc.height());

    }else if (d->mirrorHorizontaly){
        dab->mirror(true, false);
        bltFixed(mirrorX, y, dab, 0,0,rc.width(),rc.height());
    }else if (d->mirrorVerticaly){
        dab->mirror(false, true);
        bltFixed(x, mirrorY, dab, 0,0,rc.width(),rc.height());
    }

}

QVector<QRect> KisPainter::regionsRenderMirrorMask(QRect rc, KisFixedPaintDeviceSP dab)
{
    QVector<QRect> region;
    int x = rc.topLeft().x();
    int y = rc.topLeft().y();

    int mirrorX = -((x+rc.width()) - d->axisCenter.x()) + d->axisCenter.x();
    int mirrorY = -((y+rc.height()) - d->axisCenter.y()) + d->axisCenter.y();

    if (d->mirrorHorizontaly && d->mirrorVerticaly){
        dab->mirror(true, false);
        bltFixed(mirrorX, y, dab, 0,0,rc.width(),rc.height());
        region.append( QRect(QPoint(mirrorX,y),rc.size()) );

        dab->mirror(false,true);
        bltFixed(mirrorX, mirrorY, dab, 0,0,rc.width(),rc.height());
        region.append( QRect(QPoint(mirrorX,mirrorY),rc.size()) );

        dab->mirror(true, false);
        bltFixed(x, mirrorY, dab, 0,0,rc.width(),rc.height());
        region.append( QRect(QPoint(x,mirrorY),rc.size()) );

    }else if (d->mirrorHorizontaly){
        dab->mirror(true, false);
        bltFixed(mirrorX, y, dab, 0,0,rc.width(),rc.height());
        region.append( QRect(QPoint(mirrorX,y),rc.size()) );
    }else if (d->mirrorVerticaly){
        dab->mirror(false, true);
        bltFixed(x, mirrorY, dab, 0,0,rc.width(),rc.height());
        region.append( QRect(QPoint(x,mirrorY),rc.size()) );
    }
    return region;
}


void KisPainter::renderMirrorMask(QRect rc, KisFixedPaintDeviceSP dab, KisFixedPaintDeviceSP mask)
{
    int x = rc.topLeft().x();
    int y = rc.topLeft().y();

    int mirrorX = -((x+rc.width()) - d->axisCenter.x()) + d->axisCenter.x();
    int mirrorY = -((y+rc.height()) - d->axisCenter.y()) + d->axisCenter.y();

    if (d->mirrorHorizontaly && d->mirrorVerticaly){
        dab->mirror(true, false);
        mask->mirror(true, false);
        bltFixedWithFixedSelection(mirrorX,y, dab, mask, rc.width() ,rc.height() );

        dab->mirror(false,true);
        mask->mirror(false, true);
        bltFixedWithFixedSelection(mirrorX,mirrorY, dab, mask, rc.width() ,rc.height() );

        dab->mirror(true, false);
        mask->mirror(true, false);
        bltFixedWithFixedSelection(x,mirrorY, dab, mask, rc.width() ,rc.height() );

    }else if (d->mirrorHorizontaly){
        dab->mirror(true, false);
        mask->mirror(true, false);
        bltFixedWithFixedSelection(mirrorX,y, dab, mask, rc.width() ,rc.height() );

    }else if (d->mirrorVerticaly){
        dab->mirror(false, true);
        mask->mirror(false, true);
        bltFixedWithFixedSelection(x,mirrorY, dab, mask, rc.width() ,rc.height() );
    }

}


void KisPainter::renderMirrorMask(QRect rc, KisPaintDeviceSP dab){
    if (d->mirrorHorizontaly || d->mirrorVerticaly){
        KisFixedPaintDeviceSP mirrorDab = new KisFixedPaintDevice(device()->colorSpace());
        QRect dabRc( QPoint(0,0), QSize(rc.width(),rc.height()) );
        mirrorDab->setRect(dabRc);
        mirrorDab->initialize();

        dab->readBytes(mirrorDab->data(),rc);

        renderMirrorMask( QRect(rc.topLeft(),dabRc.size()), mirrorDab);
    }
}

void KisPainter::renderMirrorMask(QRect rc, KisPaintDeviceSP dab, KisFixedPaintDeviceSP mask){
    if (d->mirrorHorizontaly || d->mirrorVerticaly){
        KisFixedPaintDeviceSP mirrorDab = new KisFixedPaintDevice(device()->colorSpace());
        QRect dabRc( QPoint(0,0), QSize(rc.width(),rc.height()) );
        mirrorDab->setRect(dabRc);
        mirrorDab->initialize();
        dab->readBytes(mirrorDab->data(),rc);
        renderMirrorMask(rc, mirrorDab, mask);
    }
}

void KisPainter::renderMirrorMask(QRect rc, KisPaintDeviceSP dab, int sx, int sy, KisFixedPaintDeviceSP mask)
{
    if (d->mirrorHorizontaly || d->mirrorVerticaly){
        KisFixedPaintDeviceSP mirrorDab = new KisFixedPaintDevice(device()->colorSpace());
        QRect dabRc( QPoint(0,0), QSize(rc.width(),rc.height()) );
        mirrorDab->setRect(dabRc);
        mirrorDab->initialize();
        dab->readBytes(mirrorDab->data(),QRect(QPoint(sx,sy),rc.size()));
        renderMirrorMask(rc, mirrorDab, mask);
    }
}

