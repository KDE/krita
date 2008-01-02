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
#include <QStringList>
#include <QUndoCommand>

#include <kis_debug.h>
#include <klocale.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KoColorSpace.h>
#include <KoColor.h>

#include "kis_brush.h"
#include "kis_complex_color.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_filter.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_iterators_pixel.h"
#include "kis_random_accessor.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"

#include "kis_pixel_selection.h"

// Maximum distance from a Bezier control point to the line through the start
// and end points for the curve to be considered flat.
#define BEZIER_FLATNESS_THRESHOLD 0.5

struct KisPainter::Private {
    KisPaintDeviceSP device;
    KisSelectionSP selection;
    KisTransaction  *transaction;
    KoUpdater * progressUpdater;

    QRegion dirtyRegion;
    QRect dirtyRect;

    QRect bounds;
    KoColor paintColor;
    KoColor backgroundColor;
    KoColor fillColor;
    KisPaintLayer *sourceLayer;
    KisComplexColor *complexColor;
    FillStyle fillStyle;
    StrokeStyle strokeStyle;
    bool antiAliasPolygonFill;
    KisBrush *brush;
    KisPattern *pattern;
    QPointF duplicateOffset;
    quint8 opacity;
    KisPaintOp * paintOp;
    double pressure;
    qint32 pixelSize;
    const KoColorSpace * colorSpace;
    KoColorProfile *  profile;
    KisPaintDeviceSP dab;
    const KoCompositeOp * compositeOp;
    QBitArray channelFlags;
    bool useBoundingDirtyRect;
    KoAbstractGradient* gradient;
};

KisPainter::KisPainter()
    : d(new Private)
{
    init();
}

KisPainter::KisPainter( KisPaintDeviceSP device )
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
    d->brush = 0;
    d->pattern= 0;
    d->opacity = OPACITY_OPAQUE;
    d->dab = 0;
    d->sourceLayer = 0;
    d->complexColor = 0;
    d->fillStyle = FillStyleNone;
    d->strokeStyle = StrokeStyleBrush;
    d->pressure = PRESSURE_MIN;
    d->antiAliasPolygonFill = true;
    d->bounds = QRect();
    d->progressUpdater = 0;
    d->gradient = 0;

    KConfigGroup cfg = KGlobal::config()->group("");
    d->useBoundingDirtyRect = cfg.readEntry("aggregate_dirty_regions", true);
}

KisPainter::~KisPainter()
{
    end();
    d->brush = 0;
    delete d->paintOp;
    delete d;
}

void KisPainter::begin( KisPaintDeviceSP device )
{
    begin( device, d->selection );
}

void KisPainter::begin( KisPaintDeviceSP device, KisSelectionSP selection )
{
    if (!device) return;
    d->selection = selection;
    Q_ASSERT( device->colorSpace() );

    if (d->transaction) {
        delete d->transaction;
    }

    d->device = device;
    d->colorSpace = device->colorSpace();
    d->compositeOp = d->colorSpace->compositeOp( COMPOSITE_OVER );
    d->pixelSize = device->pixelSize();
}




QUndoCommand *KisPainter::end()
{
    return endTransaction();
}

void KisPainter::beginTransaction(const QString& customName)
{
    if (d->transaction) {
        delete d->transaction;
    }
    d->transaction = new KisTransaction(customName, d->device);
    Q_CHECK_PTR(d->transaction);
    d->device->connect( d->device.data(), SIGNAL( painterlyOverlayCreated() ), d->transaction, SLOT( painterlyOverlayAdded() ) );
}

void KisPainter::beginTransaction( KisTransaction* command)
{
    if (d->transaction) {
        delete d->transaction;
    }
    d->transaction = command;
    d->device->connect( d->device.data(), SIGNAL( painterlyOverlayCreated() ), d->transaction, SLOT( painterlyOverlayAdded() ) );
}


QUndoCommand* KisPainter::endTransaction()
{
    if ( !d->transaction ) return 0;

    if ( d->device )
            d->device->disconnect( d->transaction );

    QUndoCommand *command = d->transaction;
    d->transaction = 0;
    return command;
}

QRegion KisPainter::dirtyRegion()
{
    if ( d->useBoundingDirtyRect ) {
        QRegion r ( d->dirtyRect );
        d->dirtyRegion = QRegion();
        d->dirtyRect = QRect();
        return r;
    }
    else {
        QRegion r = d->dirtyRegion;
        d->dirtyRegion = QRegion();
        return r;
    }
}


QRegion KisPainter::addDirtyRect(QRect r)
{
    Q_ASSERT(r.width() >= 0 || r.height() >= 0);
    if ( d->useBoundingDirtyRect ) {
        d->dirtyRect = d->dirtyRect.united( r );
        return QRegion( d->dirtyRect );
    }
    else {
        d->dirtyRegion += QRegion( r );
        return d->dirtyRegion;
    }
}


void KisPainter::setPaintOp(KisPaintOp * paintOp)
{
    delete d->paintOp;
    d->paintOp = paintOp;
}

void KisPainter::bitBlt(qint32 dx, qint32 dy,
            const KoCompositeOp* op,
            const KisPaintDeviceSP src,
            qint32 sx, qint32 sy,
            qint32 sw, qint32 sh)
{
    bitBlt(dx, dy, op, src, OPACITY_OPAQUE, sx, sy, sw, sh);
}

void KisPainter::bitBlt(qint32 dx, qint32 dy,
            const QString & op,
            const KisPaintDeviceSP src,
            quint8 opacity,
            qint32 sx, qint32 sy,
            qint32 sw, qint32 sh)
{
    bitBlt(dx, dy, d->colorSpace->compositeOp(op), src, opacity, sx, sy, sw, sh);
}

void KisPainter::bitBlt(QPoint pos, const KisPaintDeviceSP src, QRect srcRect )
{
    bitBlt( pos.x(), pos.y(), d->compositeOp, src, d->opacity, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height() );
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

    srcRect &= srcdev->extent();
    
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

    const KoColorSpace * srcCs = srcdev->colorSpace();

    qint32 dstY = dy;
    qint32 srcY = sy;
    qint32 rowsRemaining = sh;

   KisRandomConstAccessorPixel srcIt = srcdev->createRandomConstAccessor(sx,sy);
   KisRandomAccessorPixel dstIt = d->device->createRandomAccessor(dx,dy);

    while (rowsRemaining > 0) {

        qint32 dstX = dx;
        qint32 srcX = sx;
        qint32 columnsRemaining = sw;
        qint32 numContiguousDstRows = d->device->numContiguousRows(dstY, dstX, dstX + sw - 1);
        qint32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);

        qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
        rows = qMin(rows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
            qint32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);

            qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
            columns = qMin(columns, columnsRemaining);

            qint32 srcRowStride = srcdev->rowStride(srcX, srcY);
            srcIt.moveTo(srcX, srcY);
            const quint8 *srcData = srcIt.rawData();

            qint32 dstRowStride = d->device->rowStride(dstX, dstY);
            dstIt.moveTo(dstX, dstY);
            quint8 *dstData = dstIt.rawData();

            d->colorSpace->bitBlt(dstData,
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
                                 d->channelFlags);

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }
}

void KisPainter::bltSelection(qint32 dx, qint32 dy,
                    const QString & op,
                    const KisPaintDeviceSP src,
                    quint8 opacity,
                    qint32 sx, qint32 sy,
                    qint32 sw, qint32 sh)
{
    bltSelection(dx, dy, d->colorSpace->compositeOp(op), src, opacity, sx, sy, sw, sh);
}

void KisPainter::bltSelection(qint32 dx, qint32 dy,
                    const KoCompositeOp *op,
                    const KisPaintDeviceSP srcdev,
                    quint8 opacity,
                    qint32 sx, qint32 sy,
                    qint32 sw, qint32 sh)
{
    if (d->device.isNull()) return;
    if ( !d->selection ) {
        bitBlt(dx, dy, op, srcdev, opacity, sx, sy, sw, sh);
    }
    else {
        bltSelection(dx, dy, op, srcdev, d->selection, opacity, sx, sy, sw, sh );
    }
}

void KisPainter::bltSelection(QPoint pos, const KisPaintDeviceSP src, const KisSelectionSP selDev, QRect srcRect )
{
    bltSelection( pos.x(), pos.y(), d->compositeOp, src, selDev, d->opacity, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height() );
}

void KisPainter::bltSelection(qint32 dx, qint32 dy,
                    const QString & op,
                    const KisPaintDeviceSP src,
                    const KisSelectionSP selMask,
                    quint8 opacity,
                    qint32 sx, qint32 sy,
                    qint32 sw, qint32 sh)
{
    bltSelection(dx, dy, d->colorSpace->compositeOp(op), src, selMask, opacity, sx, sy, sw, sh);
}

void KisPainter::bltSelection(qint32 dx, qint32 dy,
                              const KoCompositeOp  *op,
                              const KisPaintDeviceSP srcdev,
                              const KisSelectionSP selMask,
                              quint8 opacity,
                              qint32 sx, qint32 sy,
                              qint32 sw, qint32 sh)
{

    if (srcdev.isNull()) return;

    if (selMask.isNull()) return;

    if (d->device.isNull()) return;
        
    if (selMask->isProbablyTotallyUnselected(QRect(dx, dy, sw, sh))) {
        return;
    }
    QRect srcRect = QRect(sx, sy, sw, sh);
    srcRect &= srcdev->exactBounds();
    srcRect &= selMask->selectedExactRect();
    if (srcRect.isEmpty()) {
        return;
    }

    dx += srcRect.x() - sx;
    dy += srcRect.y() - sy;

    sx = srcRect.x();
    sy = srcRect.y();
    sw = srcRect.width();
    sh = srcRect.height();

    const KoColorSpace * srcCs = srcdev->colorSpace();

    qint32 dstY = dy;
    qint32 srcY = sy;
    qint32 rowsRemaining = sh;

    KisRandomConstAccessorPixel srcIt = srcdev->createRandomConstAccessor(sx,sy);
    KisRandomAccessorPixel dstIt = d->device->createRandomAccessor(dx,dy);
    KisRandomConstAccessorPixel maskIt = selMask->createRandomConstAccessor(dx,dy);
    
    while (rowsRemaining > 0) {

        qint32 dstX = dx;
        qint32 srcX = sx;
        qint32 columnsRemaining = sw;
        qint32 numContiguousDstRows = d->device->numContiguousRows(dstY, dstX, dstX + sw - 1);
        qint32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);
        qint32 numContiguousSelRows = selMask->numContiguousRows(srcY, srcX, srcX + sw - 1);
        
        qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
        rows = qMin(rows, numContiguousSelRows);
        rows = qMin(rows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
            qint32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);
            qint32 numContiguousSelColumns = selMask->numContiguousColumns(srcX, srcY, srcY + rows - 1);
            
            qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
            columns = qMin(columns, numContiguousSelColumns);
            columns = qMin(columns, columnsRemaining);

            qint32 srcRowStride = srcdev->rowStride(srcX, srcY);
            srcIt.moveTo(srcX, srcY);
            const quint8 *srcData = srcIt.rawData();

            qint32 dstRowStride = d->device->rowStride(dstX, dstY);
            dstIt.moveTo(dstX, dstY);
            quint8 *dstData = dstIt.rawData();

            qint32 maskRowStride = selMask->rowStride(dstX, dstY);
            maskIt.moveTo(dstX, dstY);
            const quint8 *maskData = maskIt.rawData();
            
            d->colorSpace->bitBlt(dstData,
                                 dstRowStride,
                                 srcCs,
                                 srcData,
                                 srcRowStride,
                                 maskData,
                                 maskRowStride,
                                 opacity,
                                 rows,
                                 columns,
                                 op,
                                 d->channelFlags);

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }
    addDirtyRect(QRect(dx, dy, sw, sh));
}


double KisPainter::paintLine(const KisPaintInformation &pi1,
                             const KisPaintInformation &pi2,
                             double savedDist)
{
    if (!d->device) return 0;
    if (!d->paintOp) return 0;
    if (!d->brush) return 0;
    
    return d->paintOp->paintLine(pi1, pi2, savedDist);
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
        paintLine (points [index], points [index + 1]);
    }
}

void KisPainter::getBezierCurvePoints(const QPointF &pos1,
                                      const QPointF &control1,
                                      const QPointF &control2,
                                      const QPointF &pos2,
                                      vQPointF& points) const
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

double KisPainter::paintBezierCurve(const KisPaintInformation &pi1,
                                    const QPointF &control1,
                                    const QPointF &control2,
                                    const KisPaintInformation &pi2,
                                    const double savedDist)
{
    return d->paintOp->paintBezierCurve(pi1, control1, control2, pi2, savedDist);
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

void KisPainter::paintAt(const KisPaintInformation& pi)
{
    if (!d->paintOp) return;
    d->paintOp->paintAt(pi);
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
            double distance = -1;

            for (int i = 0; i < points.count() - 1; i++) {
                distance = paintLine(KisPaintInformation(points[i]), KisPaintInformation(points[i + 1]), distance);
            }
            paintLine(points[points.count() - 1], points[0], distance);
        }
    }
}

void KisPainter::paintPainterPath(const QPainterPath& path)
{
    QPointF lastPoint, nextPoint;
    int elementCount = path.elementCount();
    for( int i = 0; i < elementCount; i++ )
    {
        QPainterPath::Element element = path.elementAt( i );
        switch( element.type )
        {
            case QPainterPath::MoveToElement:
                lastPoint =  QPointF( element.x, element.y );
                break;
            case QPainterPath::LineToElement:
                nextPoint =  QPointF( element.x, element.y );
                paintLine( KisPaintInformation( lastPoint ), KisPaintInformation( nextPoint ));
                lastPoint = nextPoint;
                break;
            case QPainterPath::CurveToElement:
                nextPoint =  QPointF( path.elementAt(i+2).x, path.elementAt(i+2).y );
                paintBezierCurve( KisPaintInformation( lastPoint ),
                                  QPointF( path.elementAt(i).x, path.elementAt(i).y),
                                  QPointF( path.elementAt(i+1).x, path.elementAt(i+1).y),
                                  KisPaintInformation( nextPoint ));
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

    KisPaintDeviceSP polygon = new KisPaintDevice(d->device->colorSpace(), "polygon");
    Q_CHECK_PTR(polygon);

    KisFillPainter fillPainter(polygon);

    QRectF boundingRect = path.boundingRect();
    QRect fillRect;

    fillRect.setLeft((qint32)floor(boundingRect.left()));
    fillRect.setRight((qint32)ceil(boundingRect.right()));
    fillRect.setTop((qint32)floor(boundingRect.top()));
    fillRect.setBottom((qint32)ceil(boundingRect.bottom()));

    // Expand the rectangle to allow for anti-aliasing.
    fillRect.adjust(-1, -1, 1, 1);

    // Clip to the image bounds.
    if ( d->bounds.isValid() ) {
        fillRect &= d->bounds;
    }

    switch (fillStyle) {
    default:
        // Fall through
    case FillStyleGradient:
        // Currently unsupported, fall through
    case FillStyleStrokes:
        // Currently unsupported, fall through
        kWarning(41001) << "Unknown or unsupported fill style in fillPolygon\n";
    case FillStyleForegroundColor:
        fillPainter.fillRect(fillRect, paintColor(), OPACITY_OPAQUE);
        break;
    case FillStyleBackgroundColor:
        fillPainter.fillRect(fillRect, backgroundColor(), OPACITY_OPAQUE);
        break;
    case FillStylePattern:
        Q_ASSERT(d->pattern != 0);
        fillPainter.fillRect(fillRect, d->pattern);
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
            maskPainter.fillPath(path, QColor(OPACITY_OPAQUE, OPACITY_OPAQUE, OPACITY_OPAQUE, 255));
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

    bltSelection(r.x(), r.y(), d->compositeOp, polygon, opacity(), r.x(), r.y(), r.width(), r.height());
}

void KisPainter::setProgress(KoUpdater * progressUpdater)
{
    d->progressUpdater = progressUpdater;
}

KisTransaction  * KisPainter::transaction() { return d->transaction; }

const KisPaintDeviceSP KisPainter::device() const { return d->device; }
KisPaintDeviceSP KisPainter::device() { return d->device; }

void KisPainter::setChannelFlags( QBitArray channelFlags )
{
    d->channelFlags = channelFlags;
}

QBitArray KisPainter::channelFlags()
{
    return d->channelFlags;
}

void KisPainter::setBrush(KisBrush* brush) { d->brush = brush; }
KisBrush * KisPainter::brush() const { return d->brush; }

void KisPainter::setPattern(KisPattern * pattern) { d->pattern = pattern; }
KisPattern * KisPainter::pattern() const { return d->pattern; }

void KisPainter::setPaintColor(const KoColor& color) { d->paintColor = color;}
KoColor KisPainter::paintColor() const { return d->paintColor; }

void KisPainter::setBackgroundColor(const KoColor& color) {d->backgroundColor = color; }
KoColor KisPainter::backgroundColor() const { return d->backgroundColor; }

void KisPainter::setFillColor(const KoColor& color) { d->fillColor = color; }
KoColor KisPainter::fillColor() const { return d->fillColor; }

void KisPainter::setComplexColor(KisComplexColor *color) { d->complexColor = color; }
KisComplexColor *KisPainter::complexColor() { return d->complexColor; }

void KisPainter::setFillStyle(FillStyle fillStyle) { d->fillStyle = fillStyle; }

KisPainter::FillStyle KisPainter::fillStyle() const
{
    return d->fillStyle;
}

void KisPainter::setAntiAliasPolygonFill(bool antiAliasPolygonFill) { d->antiAliasPolygonFill = antiAliasPolygonFill; }

bool KisPainter::antiAliasPolygonFill() { return d->antiAliasPolygonFill; }

void KisPainter::setStrokeStyle(KisPainter::StrokeStyle strokeStyle)
{
    d->strokeStyle = strokeStyle;
}
KisPainter::StrokeStyle KisPainter::strokeStyle() const
{
    return d->strokeStyle;
}

void KisPainter::setOpacity(quint8 opacity) { d->opacity = opacity; }
quint8 KisPainter::opacity() const { return d->opacity; }

void KisPainter::setPressure(double pressure) { d->pressure = pressure; }
double KisPainter::pressure() { return d->pressure; }

void KisPainter::setBounds( const QRect & bounds ) { d->bounds = bounds;  }
QRect KisPainter::bounds() { return d->bounds;  }

KisPaintOp * KisPainter::paintOp() const { return d->paintOp; }

void KisPainter::setDab(KisPaintDeviceSP dab) { d->dab = dab; }
KisPaintDeviceSP KisPainter::dab() const { return d->dab; }

void KisPainter::setCompositeOp(const KoCompositeOp * op) { d->compositeOp = op; }
const KoCompositeOp * KisPainter::compositeOp() { return d->compositeOp; }

void KisPainter::setSelection(KisSelectionSP selection) { d->selection = selection; }

KisSelectionSP KisPainter::selection() { return d->selection; }

KoUpdater * KisPainter::progressUpdater() { return d->progressUpdater; }

void KisPainter::setGradient(KoAbstractGradient* gradient)
{
    d->gradient = gradient;
}
KoAbstractGradient* KisPainter::gradient()
{
    return d->gradient;
}
