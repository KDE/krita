/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <cmath>
#include <climits>
#include <strings.h>

#include <qregion.h>
#include <QImage>
#include <QRect>
#include <QString>
#include <QUndoCommand>

#include <kdebug.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "KoColorSpace.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "KoColor.h"

// Maximum distance from a Bezier control point to the line through the start
// and end points for the curve to be considered flat.
#define BEZIER_FLATNESS_THRESHOLD 0.5

KisPainter::KisPainter()
{
    init();
}

KisPainter::KisPainter(KisPaintDeviceSP device)
{
    init();
    Q_ASSERT(device);
    begin(device);
}

void KisPainter::init()
{
    m_transaction = 0;
    m_paintOp = 0;
    m_filter = 0;
    m_brush = 0;
    m_pattern= 0;
    m_opacity = OPACITY_OPAQUE;
    m_dab = 0;
    m_fillStyle = FillStyleNone;
    m_strokeStyle = StrokeStyleBrush;
    m_pressure = PRESSURE_MIN;
    m_antiAliasPolygonFill = true;
}

KisPainter::~KisPainter()
{
    m_brush = 0;
    delete m_paintOp;
    end();
}

void KisPainter::begin(KisPaintDeviceSP device)
{
    if (!device) return;

    if (m_transaction)
        delete m_transaction;

    m_device = device;
    m_colorSpace = device->colorSpace();
    m_compositeOp = m_colorSpace->compositeOp( COMPOSITE_OVER );
    m_pixelSize = device->pixelSize();
}

QUndoCommand *KisPainter::end()
{
    return endTransaction();
}

void KisPainter::beginTransaction(const QString& customName)
{
    if (m_transaction)
        delete m_transaction;
    m_transaction = new KisTransaction(customName, m_device);
    Q_CHECK_PTR(m_transaction);
}

void KisPainter::beginTransaction( KisTransaction* command)
{
    if (m_transaction)
        delete m_transaction;
    m_transaction = command;
}


QUndoCommand *KisPainter::endTransaction()
{
    QUndoCommand *command = m_transaction;
    m_transaction = 0;
    return command;
}

QRegion KisPainter::dirtyRegion()
{
    QRegion r = m_dirtyRegion;

    m_dirtyRegion = QRegion();

    return r;
}


QRegion KisPainter::addDirtyRect(QRect r)
{
    m_dirtyRegion += QRegion( r );
    return m_dirtyRegion;
}

void KisPainter::bitBlt(qint32 dx, qint32 dy,
                        const KoCompositeOp* op,
                        const QImage * src,
                        quint8 opacity,
                        qint32 sx, qint32 sy,
                        qint32 sw, qint32 sh)
{
#ifdef __GNUC__
#warning "Don't assume the same resolution for a QImage and a KisPaintDevice -- see QImage::dotsPerMeterX|Y"
#endif

    if ( src == 0 ) return;
    if ( src->isNull() ) return;
    if ( src->format() != QImage::Format_RGB32 && src->format() != QImage::Format_ARGB32 ) return;
    if ( src->depth() != 32 ) return;

    QRect srcRect = QRect( sx, sy, sw, sh );
    KoColorSpace * srcCs = KoColorSpaceRegistry::instance()->rgb8();

    // Make sure we don't try to blit outside the source image
    if ( src->rect().isValid() && op != srcCs->compositeOp( COMPOSITE_COPY ) ) {
        srcRect &= src->rect();
    }

    dx += srcRect.x() - sx;
    dy += srcRect.y() - sy;

    sx = srcRect.x();
    sy = srcRect.y();
    sw = srcRect.width();
    sh = srcRect.height();

    addDirtyRect( QRect( dx, dy, sw, sh ));

    const quint8 * srcData = src->bits();

    for ( qint32 row = 0; row < sh; ++row ) {

        KisHLineIteratorPixel dstIt = m_device->createHLineIterator(dx, dy + row, sw);
        while ( !dstIt.isDone() ) {
            qint32 pixels = dstIt.nConseqHPixels();
            m_colorSpace->bitBlt(dstIt.rawData(),
                                 0,
                                 srcCs,
                                 srcData,
                                 0,
                                 0,
                                 0,
                                 opacity,
                                 1,
                                 pixels,
                                 op,
                                 m_channelFlags);

            srcData += ( pixels * 4 ); // 4 bytes to one QImage pixel

            dstIt += pixels;

        }
        dstIt.nextRow();

    }

}

void KisPainter::bitBlt(QPoint pos, const QImage * src, QRect srcRect )
{
    bitBlt( pos.x(), pos.y(), m_compositeOp, src, m_opacity, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height() );
}

void KisPainter::bitBlt(qint32 dx, qint32 dy,
                        const KoCompositeOp* op,
                        const KisPaintDeviceSP srcdev,
                        quint8 opacity,
                        qint32 sx, qint32 sy,
                        qint32 sw, qint32 sh)
{
    if (srcdev.isNull()) {
        return;
    }

    QRect srcRect = QRect(sx, sy, sw, sh);

    if (srcdev->extentIsValid() && op != srcdev->colorSpace()->compositeOp( COMPOSITE_COPY )) {
        srcRect &= srcdev->extent();
    }

    if (srcRect.isEmpty()) {
        return;
    }

    dx += srcRect.x() - sx;
    dy += srcRect.y() - sy;

    sx = srcRect.x();
    sy = srcRect.y();
    sw = srcRect.width();
    sh = srcRect.height();

    addDirtyRect( QRect( dx, dy, sw, sh ) );

    KoColorSpace * srcCs = srcdev->colorSpace();

    qint32 dstY = dy;
    qint32 srcY = sy;
    qint32 rowsRemaining = sh;

    while (rowsRemaining > 0) {

        qint32 dstX = dx;
        qint32 srcX = sx;
        qint32 columnsRemaining = sw;
        qint32 numContiguousDstRows = m_device->numContiguousRows(dstY, dstX, dstX + sw - 1);
        qint32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);

        qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
        rows = qMin(rows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = m_device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
            qint32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);

            qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
            columns = qMin(columns, columnsRemaining);

            qint32 srcRowStride = srcdev->rowStride(srcX, srcY);
            KisHLineConstIteratorPixel srcIt = srcdev->createHLineConstIterator(srcX, srcY, columns);
            const quint8 *srcData = srcIt.rawData();

            qint32 dstRowStride = m_device->rowStride(dstX, dstY);
            KisHLineIteratorPixel dstIt = m_device->createHLineIterator(dstX, dstY, columns);
            quint8 *dstData = dstIt.rawData();

            m_colorSpace->bitBlt(dstData,
                                 dstRowStride,
                                 srcCs,
                                 srcData,
                                 srcRowStride,
                                 0,
                                 0,
                                 opacity,
                                 rows,
                                 columns,
                                 op,
                                 m_channelFlags);

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }
}

void KisPainter::bitBlt(QPoint pos, const KisPaintDeviceSP src, QRect srcRect )
{
    bitBlt( pos.x(), pos.y(), m_compositeOp, src, m_opacity, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height() );
}

void KisPainter::bltMask(qint32 dx, qint32 dy,
                         const KoCompositeOp *op,
                         const KisPaintDeviceSP srcdev,
                         const KisPaintDeviceSP selMask,
                         quint8 opacity,
                         qint32 sx, qint32 sy,
                         qint32 sw, qint32 sh)
{
    if (srcdev.isNull()) return;

    if (selMask.isNull()) return;

    if (m_device.isNull()) return;


    QRect srcRect = QRect(sx, sy, sw, sh);

    if (srcdev->extentIsValid() && op != srcdev->colorSpace()->compositeOp( COMPOSITE_COPY )) {
        srcRect &= srcdev->extent();
    }

    if (srcRect.isEmpty()) {
        return;
    }

    dx += srcRect.x() - sx;
    dy += srcRect.y() - sy;

    sx = srcRect.x();
    sy = srcRect.y();
    sw = srcRect.width();
    sh = srcRect.height();

    addDirtyRect(QRect(dx, dy, sw, sh));

    KoColorSpace * srcCs = srcdev->colorSpace();

    qint32 dstY = dy;
    qint32 srcY = sy;
    qint32 rowsRemaining = sh;

    while (rowsRemaining > 0) {

        qint32 dstX = dx;
        qint32 srcX = sx;
        qint32 columnsRemaining = sw;
        qint32 numContiguousDstRows = m_device->numContiguousRows(dstY, dstX, dstX + sw - 1);
        qint32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);
        qint32 numContiguousSelRows = selMask->numContiguousRows(dstY, dstX, dstX + sw - 1);

        qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
        rows = qMin(numContiguousSelRows, rows);
        rows = qMin(rows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = m_device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
            qint32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);
            qint32 numContiguousSelColumns = selMask->numContiguousColumns(dstX, dstY, dstY + rows - 1);

            qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
            columns = qMin(numContiguousSelColumns, columns);
            columns = qMin(columns, columnsRemaining);

            qint32 dstRowStride = m_device->rowStride(dstX, dstY);
            KisHLineIteratorPixel dstIt = m_device->createHLineIterator(dstX, dstY, columns);
            quint8 *dstData = dstIt.rawData();

            qint32 srcRowStride = srcdev->rowStride(srcX, srcY);
            KisHLineConstIteratorPixel srcIt = srcdev->createHLineConstIterator(srcX, srcY, columns);
            const quint8 *srcData = srcIt.rawData();

            qint32 selRowStride = selMask->rowStride(dstX, dstY);
            KisHLineConstIteratorPixel selIt = selMask->createHLineConstIterator(dstX, dstY, columns);
            const quint8 *selData = selIt.rawData();

            m_colorSpace->bitBlt(dstData,
                                 dstRowStride,
                                 srcCs,
                                 srcData,
                                 srcRowStride,
                                 selData,
                                 selRowStride,
                                 opacity,
                                 rows,
                                 columns,
                                 op,
                                 m_channelFlags);

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }
}

void KisPainter::bltMask(QPoint pos, const KisPaintDeviceSP src, KisPaintDeviceSP selMask, QRect srcRect )
{
    bltMask( pos.x(), pos.y(), m_compositeOp, src, selMask, m_opacity, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height() );
}

void KisPainter::bltSelection(qint32 dx, qint32 dy,
                              const KoCompositeOp * op,
                              const KisPaintDeviceSP srcdev,
                              const KisSelectionSP seldev,
                              quint8 opacity,
                              qint32 sx, qint32 sy,
                              qint32 sw, qint32 sh)
{
    if (!seldev) return;
    // Better use a probablistic method than a too slow one
    if (seldev->isProbablyTotallyUnselected(QRect(dx, dy, sw, sh))) {
        return;
    }
    bltMask(dx, dy, op, srcdev, seldev, opacity, sx, sy, sw, sh);

}

void KisPainter::bltSelection(QPoint pos, const KisPaintDeviceSP src, const KisSelectionSP selDev, QRect srcRect )
{
    bltSelection( pos.x(), pos.y(), m_compositeOp, src, selDev, m_opacity, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height() );
}


void KisPainter::bltSelection(qint32 dx, qint32 dy,
                              const KoCompositeOp* op,
                              const KisPaintDeviceSP srcdev,
                              quint8 opacity,
                              qint32 sx, qint32 sy,
                              qint32 sw, qint32 sh)
{
    if (m_device.isNull()) return;
    if (!m_device->hasSelection()) {
        bitBlt(dx, dy, op, srcdev, opacity, sx, sy, sw, sh);
    }
    else
        bltSelection(dx,dy,op,srcdev, m_device->selection(),opacity,sx,sy,sw,sh);
}

double KisPainter::paintLine(const QPointF & pos1,
                             const double pressure1,
                             const double xTilt1,
                             const double yTilt1,
                             const QPointF & pos2,
                             const double pressure2,
                             const double xTilt2,
                             const double yTilt2,
                             const double inSavedDist)
{
    if (!m_device) return 0;
    if (!m_paintOp) return 0;
    if (!m_brush) return 0;

    double savedDist = inSavedDist;
    KisVector2D end(pos2);
    KisVector2D start(pos1);

    KisVector2D dragVec = end - start;
    KisVector2D movement = dragVec;

    if (savedDist < 0) {
        m_paintOp->paintAt(pos1, KisPaintInformation(pressure1, xTilt1, yTilt1, movement));
        savedDist = 0;
    }

    // XXX: The spacing should vary as the pressure changes along the line.
    // This is a quick simplification.
    double xSpacing = m_brush->xSpacing((pressure1 + pressure2) / 2);
    double ySpacing = m_brush->ySpacing((pressure1 + pressure2) / 2);

    if (xSpacing < 0.5) {
        xSpacing = 0.5;
    }
    if (ySpacing < 0.5) {
        ySpacing = 0.5;
    }

    double xScale = 1;
    double yScale = 1;
    double spacing;
    // Scale x or y so that we effectively have a square brush
    // and calculate distance in that coordinate space. We reverse this scaling
    // before drawing the brush. This produces the correct spacing in both
    // x and y directions, even if the brush's aspect ratio is not 1:1.
    if (xSpacing > ySpacing) {
        yScale = xSpacing / ySpacing;
        spacing = xSpacing;
    }
    else {
        xScale = ySpacing / xSpacing;
        spacing = ySpacing;
    }

    dragVec.setX(dragVec.x() * xScale);
    dragVec.setY(dragVec.y() * yScale);

    double newDist = dragVec.length();
    double dist = savedDist + newDist;
    double l_savedDist = savedDist;

    if (dist < spacing) {
        return dist;
    }

    dragVec.normalize();
    KisVector2D step(0, 0);

    while (dist >= spacing) {
        if (l_savedDist > 0) {
            step += dragVec * (spacing - l_savedDist);
            l_savedDist -= spacing;
        }
        else {
            step += dragVec * spacing;
        }

        QPointF p(start.x() + (step.x() / xScale), start.y() + (step.y() / yScale));

        double distanceMoved = step.length();
        double t = 0;

        if (newDist > DBL_EPSILON) {
            t = distanceMoved / newDist;
        }

        double pressure = (1 - t) * pressure1 + t * pressure2;
        double xTilt = (1 - t) * xTilt1 + t * xTilt2;
        double yTilt = (1 - t) * yTilt1 + t * yTilt2;

        m_paintOp->paintAt(p, KisPaintInformation(pressure, xTilt, yTilt, movement));
        dist -= spacing;
    }

    addDirtyRect( QRect( pos1.toPoint(), pos2.toPoint() ) );

    if (dist > 0)
        return dist;
    else
        return 0;
}

void KisPainter::paintPolyline (const vQPointF &points,
                                int index, int numPoints)
{
    if (index >= (int) points.count ())
        return;

    if (numPoints < 0)
        numPoints = points.count ();

    if (index + numPoints > (int) points.count ())
        numPoints = points.count () - index;


    for (int i = index; i < index + numPoints - 1; i++)
    {
        paintLine (points [index], 0/*pressure*/, 0, 0, points [index + 1],
                   0/*pressure*/, 0, 0);
    }
}

void KisPainter::getBezierCurvePoints(const QPointF &pos1,
                                      const QPointF &control1,
                                      const QPointF &control2,
                                      const QPointF &pos2,
                                      vQPointF& points)
{
    double d1 = pointToLineDistance(control1, pos1, pos2);
    double d2 = pointToLineDistance(control2, pos1, pos2);

    if (d1 < BEZIER_FLATNESS_THRESHOLD && d2 < BEZIER_FLATNESS_THRESHOLD) {
        points.push_back(pos1);
    } else {
        // Midpoint subdivision. See Foley & Van Dam Computer Graphics P.508
        KisVector2D p1 = pos1;
        KisVector2D p2 = control1;
        KisVector2D p3 = control2;
        KisVector2D p4 = pos2;

        KisVector2D l2 = (p1 + p2) / 2;
        KisVector2D h = (p2 + p3) / 2;
        KisVector2D l3 = (l2 + h) / 2;
        KisVector2D r3 = (p3 + p4) / 2;
        KisVector2D r2 = (h + r3) / 2;
        KisVector2D l4 = (l3 + r2) / 2;
        KisVector2D r1 = l4;
        KisVector2D l1 = p1;
        KisVector2D r4 = p4;

        getBezierCurvePoints(l1.toKoPoint(), l2.toKoPoint(), l3.toKoPoint(), l4.toKoPoint(), points);
        getBezierCurvePoints(r1.toKoPoint(), r2.toKoPoint(), r3.toKoPoint(), r4.toKoPoint(), points);
    }
}

double KisPainter::paintBezierCurve(const QPointF &pos1,
                                    const double pressure1,
                                    const double xTilt1,
                                    const double yTilt1,
                                    const QPointF &control1,
                                    const QPointF &control2,
                                    const QPointF &pos2,
                                    const double pressure2,
                                    const double xTilt2,
                                    const double yTilt2,
                                    const double savedDist)
{
    double newDistance;
    double d1 = pointToLineDistance(control1, pos1, pos2);
    double d2 = pointToLineDistance(control2, pos1, pos2);

    if (d1 < BEZIER_FLATNESS_THRESHOLD && d2 < BEZIER_FLATNESS_THRESHOLD) {
        newDistance = paintLine(pos1, pressure1, xTilt1, yTilt1, pos2, pressure2, xTilt2, yTilt2, savedDist);
    } else {
        // Midpoint subdivision. See Foley & Van Dam Computer Graphics P.508
        KisVector2D p1 = pos1;
        KisVector2D p2 = control1;
        KisVector2D p3 = control2;
        KisVector2D p4 = pos2;

        KisVector2D l2 = (p1 + p2) / 2;
        KisVector2D h = (p2 + p3) / 2;
        KisVector2D l3 = (l2 + h) / 2;
        KisVector2D r3 = (p3 + p4) / 2;
        KisVector2D r2 = (h + r3) / 2;
        KisVector2D l4 = (l3 + r2) / 2;
        KisVector2D r1 = l4;
        KisVector2D l1 = p1;
        KisVector2D r4 = p4;

        double midPressure = (pressure1 + pressure2) / 2;
        double midXTilt = (xTilt1 + xTilt2) / 2;
        double midYTilt = (yTilt1 + yTilt2) / 2;

        newDistance = paintBezierCurve(l1.toKoPoint(), pressure1, xTilt1, yTilt1,
                                       l2.toKoPoint(), l3.toKoPoint(),
                                       l4.toKoPoint(), midPressure, midXTilt, midYTilt,
                                       savedDist);
        newDistance = paintBezierCurve(r1.toKoPoint(), midPressure, midXTilt, midYTilt,
                                       r2.toKoPoint(),
                                       r3.toKoPoint(),
                                       r4.toKoPoint(), pressure2, xTilt2, yTilt2, newDistance);
    }

    return newDistance;
}

void KisPainter::paintRect(const QRectF &rect,
                           const double /*pressure*/,
                           const double /*xTilt*/,
                           const double /*yTilt*/)
{
    QRectF normalizedRect = rect.normalized();

    vQPointF points;

    points.push_back(normalizedRect.topLeft());
    points.push_back(normalizedRect.bottomLeft());
    points.push_back(normalizedRect.bottomRight());
    points.push_back(normalizedRect.topRight());

    paintPolygon(points);
}

void KisPainter::paintRect(const double x,
                           const double y,
                           const double w,
                           const double h,
                           const double pressure,
                           const double xTilt,
                           const double yTilt)
{
    paintRect(QRectF(x, y, w, h), pressure, xTilt, yTilt);
}

void KisPainter::paintEllipse(const QRectF &rect,
                              const double /*pressure*/,
                              const double /*xTilt*/,
                              const double /*yTilt*/)
{
    QRectF r = rect.normalized();

    // See http://www.whizkidtech.redprince.net/bezier/circle/ for explanation.
    // kappa = (4/3*(sqrt(2)-1))
    const double kappa = 0.5522847498;
    const double lx = (r.width() / 2) * kappa;
    const double ly = (r.height() / 2) * kappa;

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

void KisPainter::paintEllipse(const double x,
                              const double y,
                              const double w,
                              const double h,
                              const double pressure,
                              const double xTilt,
                              const double yTilt)
{
    paintEllipse(QRectF(x, y, w, h), pressure, xTilt, yTilt);
}

void KisPainter::paintAt(const QPointF & pos,
                         const double pressure,
                         const double xTilt,
                         const double yTilt)
{
    if (!m_paintOp) return;
    m_paintOp->paintAt(pos, KisPaintInformation(pressure, xTilt, yTilt, KisVector2D()));
}

double KisPainter::pointToLineDistance(const QPointF& p, const QPointF& l0, const QPointF& l1)
{
    double lineLength = sqrt((l1.x() - l0.x()) * (l1.x() - l0.x()) + (l1.y() - l0.y()) * (l1.y() - l0.y()));
    double distance = 0;

    if (lineLength > DBL_EPSILON) {
        distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / lineLength;
        distance = fabs(distance);
    }

    return distance;
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

    // Fill the polygon bounding rectangle with the required contents then we'll
    // create a mask for the actual polygon coverage.

    KisPaintDeviceSP polygon = new KisPaintDevice(m_device->colorSpace(), "polygon");
    Q_CHECK_PTR(polygon);

    KisFillPainter fillPainter(polygon);

    QRectF boundingRect = polygonPath.boundingRect();
    QRect fillRect;

    fillRect.setLeft((qint32)floor(boundingRect.left()));
    fillRect.setRight((qint32)ceil(boundingRect.right()));
    fillRect.setTop((qint32)floor(boundingRect.top()));
    fillRect.setBottom((qint32)ceil(boundingRect.bottom()));

    // Expand the rectangle to allow for anti-aliasing.
    fillRect.adjust(-1, -1, 1, 1);

    // Clip to the image bounds.
    if (m_device->image()) {
        fillRect &= m_device->image()->bounds();
    }

    switch (fillStyle) {
    default:
        // Fall through
    case FillStyleGradient:
        // Currently unsupported, fall through
    case FillStyleStrokes:
        // Currently unsupported, fall through
        kWarning(DBG_AREA_CORE) << "Unknown or unsupported fill style in fillPolygon\n";
    case FillStyleForegroundColor:
        fillPainter.fillRect(fillRect, paintColor(), OPACITY_OPAQUE);
        break;
    case FillStyleBackgroundColor:
        fillPainter.fillRect(fillRect, backgroundColor(), OPACITY_OPAQUE);
        break;
    case FillStylePattern:
        Q_ASSERT(m_pattern != 0);
        fillPainter.fillRect(fillRect, m_pattern);
        break;
    }

    KisSelectionSP polygonMask = new KisSelection(polygon);

    const qint32 MASK_IMAGE_WIDTH = 256;
    const qint32 MASK_IMAGE_HEIGHT = 256;

    QImage polygonMaskImage(MASK_IMAGE_WIDTH, MASK_IMAGE_HEIGHT, QImage::Format_ARGB32);
    QPainter maskPainter(&polygonMaskImage);
    maskPainter.setRenderHint(QPainter::Antialiasing, antiAliasPolygonFill());

    // Break the mask up into chunks so we don't have to allocate a potentially very large QImage.

    for (qint32 x = fillRect.x(); x < fillRect.x() + fillRect.width(); x += MASK_IMAGE_WIDTH) {
        for (qint32 y = fillRect.y(); y < fillRect.y() + fillRect.height(); y += MASK_IMAGE_HEIGHT) {

            maskPainter.fillRect(polygonMaskImage.rect(), QColor(OPACITY_TRANSPARENT, OPACITY_TRANSPARENT, OPACITY_TRANSPARENT, 255));
            maskPainter.translate(-x, -y);
            maskPainter.fillPath(polygonPath, QColor(OPACITY_OPAQUE, OPACITY_OPAQUE, OPACITY_OPAQUE, 255));
            maskPainter.translate(x, y);

            qint32 rectWidth = qMin(fillRect.x() + fillRect.width() - x, MASK_IMAGE_WIDTH);
            qint32 rectHeight = qMin(fillRect.y() + fillRect.height() - y, MASK_IMAGE_HEIGHT);

            KisRectIterator rectIt = polygonMask->createRectIterator(x, y, rectWidth, rectHeight);

            while (!rectIt.isDone()) {
                (*rectIt.rawData()) = qRed(polygonMaskImage.pixel(rectIt.x() - x, rectIt.y() - y));
                ++rectIt;
            }
        }
    }

    polygon->applySelectionMask(polygonMask);

    QRect r = polygon->extent();

    // The strokes for the outline may have already added updated the dirtyrect, but it can't hurt,
    // and if we're painting without outlines, then there will be no dirty rect. Let's do it ourselves...

    bltSelection(r.x(), r.y(), m_compositeOp, polygon, opacity(), r.x(), r.y(), r.width(), r.height());
}

void KisPainter::paintPolygon(const vQPointF& points)
{
    if (m_fillStyle != FillStyleNone) {
        fillPolygon(points, m_fillStyle);
    }

    if (m_strokeStyle != StrokeStyleNone) {
        if (points.count() > 1) {
            double distance = -1;

            for (int i = 0; i < points.count() - 1; i++) {
                distance = paintLine(points[i], PRESSURE_DEFAULT, 0, 0, points[i + 1], PRESSURE_DEFAULT, 0, 0, distance);
            }
            paintLine(points[points.count() - 1], PRESSURE_DEFAULT, 0, 0, points[0], PRESSURE_DEFAULT, 0, 0, distance);
        }
    }
}

