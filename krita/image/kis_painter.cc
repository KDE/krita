/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
#include <KoCompositeOp.h>

#include "kis_image.h"
#include "filter/kis_filter.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_iterators_pixel.h"
#include "kis_random_accessor.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_pixel_selection.h"
#include "kis_paint_information.h"
#include "kis_paintop_registry.h"
#include "kis_perspective_math.h"

// Maximum distance from a Bezier control point to the line through the start
// and end points for the curve to be considered flat.
#define BEZIER_FLATNESS_THRESHOLD 0.5

struct KisPainter::Private {
    KisPaintDeviceSP            device;
    KisSelectionSP              selection;
    KisTransaction*             transaction;
    KoUpdater*                  progressUpdater;

    QRegion                     dirtyRegion;
    QRect                       dirtyRect;
    KisPaintOp*                 paintOp;
    QRect                       bounds;
    KoColor                     paintColor;
    KoColor                     backgroundColor;
    KoColor                     fillColor;
    KisFilterConfiguration*     generator;
    KisPaintLayer*              sourceLayer;
    FillStyle                   fillStyle;
    StrokeStyle                 strokeStyle;
    bool                        antiAliasPolygonFill;
    KisPattern*                 pattern;
    QPointF                     duplicateOffset;
    quint8                      opacity;
    quint32                     pixelSize;
    const KoColorSpace*         colorSpace;
    KoColorProfile*             profile;
    const KoCompositeOp*        compositeOp;
    QBitArray                   channelFlags;
    bool                        useBoundingDirtyRect;
    const KoAbstractGradient*   gradient;
    KisPaintOpPresetSP          paintOpPreset;
    QImage                      polygonMaskImage;
    QPainter*                   maskPainter;
    KisFillPainter*             fillPainter;
    qint32                      maskImageWidth;
    qint32                      maskImageHeight;
    bool                        alphaLocked;
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
    d->opacity = OPACITY_OPAQUE;
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
    d->alphaLocked = false;

    KConfigGroup cfg = KGlobal::config()->group("");
    d->useBoundingDirtyRect = cfg.readEntry("aggregate_dirty_regions", true);
}

KisPainter::~KisPainter()
{
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

    delete d->transaction;
    d->transaction = 0;

    d->device = device;
    d->colorSpace = device->colorSpace();
    d->compositeOp = d->colorSpace->compositeOp(COMPOSITE_OVER);
    d->pixelSize = device->pixelSize();
}
QUndoCommand *KisPainter::end()
{
    return endTransaction();
}

void KisPainter::beginTransaction(const QString& customName)
{
    delete d->transaction;
    d->transaction = new KisTransaction(customName, d->device);
    Q_CHECK_PTR(d->transaction);
}

void KisPainter::beginTransaction(KisTransaction* command)
{
    delete d->transaction;
    d->transaction = command;
}


QUndoCommand* KisPainter::endTransaction()
{
    if (!d->transaction) return 0;

    if (d->device)
        d->device->disconnect(d->transaction);

    QUndoCommand *command = d->transaction;
    d->transaction = 0;
    return command;
}

QRegion KisPainter::dirtyRegion()
{
    if (d->useBoundingDirtyRect) {
        QRegion r(d->dirtyRect);
        d->dirtyRegion = QRegion();
        d->dirtyRect = QRect();
        return r;
    } else {
        QRegion r = d->dirtyRegion;
        d->dirtyRegion = QRegion();
        return r;
    }
}


QRegion KisPainter::addDirtyRect(const QRect & rc)
{
    QRect r = rc.normalized();

    if (!r.isValid() || r.width() <= 0 || r.height() <= 0) {
        return d->dirtyRegion;
    }

    if (d->useBoundingDirtyRect) {
        d->dirtyRect = d->dirtyRect.united(r);
        return QRegion(d->dirtyRect);
    } else {
        d->dirtyRegion += QRegion(r);
        return d->dirtyRegion;
    }
}


void KisPainter::bitBlt(qint32 dx, qint32 dy,
                        const KisPaintDeviceSP srcdev,
                        qint32 sx, qint32 sy,
                        qint32 sw, qint32 sh)
{
    if (sw == 0 || sh == 0) return;
    if (srcdev.isNull()) return;
    if (d->device.isNull()) return;

    QRect srcRect = QRect(sx, sy, sw, sh);

    // In case of COMPOSITE_COPY restricting bitblt to extent can
    // have unexpected behavior since it would reduce the area that
    // is copied.
    if (d->compositeOp->id() != COMPOSITE_COPY) {
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

    const KoColorSpace * srcCs = srcdev->colorSpace();

    qint32 dstY = dy;
    qint32 srcY = sy;
    qint32 rowsRemaining = sh;

    KisRandomConstAccessorPixel srcIt = srcdev->createRandomConstAccessor(sx, sy);
    KisRandomAccessorPixel dstIt = d->device->createRandomAccessor(dx, dy);

    if (d->selection) {

        KisRandomConstAccessorPixel maskIt = d->selection->createRandomConstAccessor(dx, dy);

        while (rowsRemaining > 0) {

            qint32 dstX = dx;
            qint32 srcX = sx;
            qint32 columnsRemaining = sw;
            qint32 numContiguousDstRows = d->device->numContiguousRows(dstY, dstX, dstX + sw - 1);
            qint32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);
            qint32 numContiguousSelRows = d->selection->numContiguousRows(srcY, srcX, srcX + sw - 1);

            qint32 rows = qMin(numContiguousDstRows, numContiguousSrcRows);
            rows = qMin(rows, numContiguousSelRows);
            rows = qMin(rows, rowsRemaining);

            while (columnsRemaining > 0) {

                qint32 numContiguousDstColumns = d->device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
                qint32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);
                qint32 numContiguousSelColumns = d->selection->numContiguousColumns(srcX, srcY, srcY + rows - 1);

                qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
                columns = qMin(columns, numContiguousSelColumns);
                columns = qMin(columns, columnsRemaining);

                qint32 srcRowStride = srcdev->rowStride(srcX, srcY);
                srcIt.moveTo(srcX, srcY);

                qint32 dstRowStride = d->device->rowStride(dstX, dstY);
                dstIt.moveTo(dstX, dstY);

                qint32 maskRowStride = d->selection->rowStride(dstX, dstY);
                maskIt.moveTo(dstX, dstY);

                d->colorSpace->bitBlt(dstIt.rawData(),
                                      dstRowStride,
                                      d->alphaLocked,
                                      srcCs,
                                      srcIt.rawData(),
                                      srcRowStride,
                                      maskIt.rawData(),
                                      maskRowStride,
                                      d->opacity,
                                      rows,
                                      columns,
                                      d->compositeOp,
                                      d->channelFlags);
                srcX += columns;
                dstX += columns;
                columnsRemaining -= columns;
            }

            srcY += rows;
            dstY += rows;
            rowsRemaining -= rows;
        }
    } else {

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

                qint32 dstRowStride = d->device->rowStride(dstX, dstY);
                dstIt.moveTo(dstX, dstY);

                d->colorSpace->bitBlt(dstIt.rawData(),
                                      dstRowStride,
                                      d->alphaLocked,
                                      srcCs,
                                      srcIt.rawData(),
                                      srcRowStride,
                                      0,
                                      0,
                                      d->opacity,
                                      rows,
                                      columns,
                                      d->compositeOp,
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

    addDirtyRect(QRect(dx, dy, sw, sh));
}

void KisPainter::bitBlt(const QPoint & pos, const KisPaintDeviceSP src, const QRect & srcRect)
{
    bitBlt(pos.x(), pos.y(), src, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}



void KisPainter::bltFixed(qint32 dx, qint32 dy,
                          const KisFixedPaintDeviceSP srcDev,
                          qint32 sx, qint32 sy,
                          qint32 sw, qint32 sh)
{
    if (sw == 0 || sh == 0) return;
    if (srcDev.isNull()) return;
    if (d->device.isNull()) return;
    Q_ASSERT(srcDev->pixelSize() == d->pixelSize);

    QRect srcRect = QRect(sx, sy, sw, sh);

    if (srcRect.isEmpty()) {
        return;
    }

    if (!srcRect.contains(srcDev->bounds())) {
        srcRect = srcDev->bounds();
    }

    dx += srcRect.x() - sx;
    dy += srcRect.y() - sy;

    sx = srcRect.x();
    sy = srcRect.y();
    sw = srcRect.width();
    sh = srcRect.height();

    const KoColorSpace * srcCs = d->colorSpace;
    quint8* dstBytes = new quint8[sw * sh * d->device->pixelSize()];
    d->device->readBytes(dstBytes, dx, dy, sw, sh);

    if (d->selection) {

        quint8* selBytes = new quint8[sw * sh * d->selection->pixelSize()];
        d->selection->readBytes(selBytes, dx, dy, sw, sh);

        d->colorSpace->bitBlt(dstBytes,
                              sw * d->device->pixelSize(),
                              d->alphaLocked,
                              srcCs,
                              srcDev->data() + sx,
                              srcDev->bounds().width() * srcDev->pixelSize(),
                              selBytes,
                              sw  * d->selection->pixelSize(),
                              d->opacity,
                              sh,
                              sw,
                              d->compositeOp,
                              d->channelFlags);

        delete[] selBytes;
    } else {
        d->colorSpace->bitBlt(dstBytes,
                              sw * d->device->pixelSize(),
                              d->alphaLocked,
                              srcCs,
                              srcDev->data() + sx,
                              srcDev->bounds().width() * srcDev->pixelSize(),
                              0,
                              0,
                              d->opacity,
                              sh,
                              sw,
                              d->compositeOp,
                              d->channelFlags);

    }

    d->device->writeBytes(dstBytes, dx, dy, sw, sh);

    delete[] dstBytes;

    addDirtyRect(QRect(dx, dy, sw, sh));
}

void KisPainter::bltFixed(const QPoint & pos, const KisFixedPaintDeviceSP src, const QRect & srcRect)
{
    bltFixed(pos.x(), pos.y(), src, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}

double KisPainter::paintLine(const KisPaintInformation &pi1,
                             const KisPaintInformation &pi2,
                             double savedDist)
{
    if (!d->device) return 0;
    if (!d->paintOp) return 0;

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


    for (int i = index; i < index + numPoints - 1; i++) {
        paintLine(points [index], points [index + 1]);
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

double KisPainter::paintBezierCurve(const KisPaintInformation &pi1,
                                    const QPointF &control1,
                                    const QPointF &control2,
                                    const KisPaintInformation &pi2,
                                    const double savedDist)
{
    if (d->paintOp) {
        return d->paintOp->paintBezierCurve(pi1, control1, control2, pi2, savedDist);
    }
    return 0.5;
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

void KisPainter::paintRect(const double x,
                           const double y,
                           const double w,
                           const double h)
{
    paintRect(QRectF(x, y, w, h));
}

void KisPainter::paintEllipse(const QRectF &rect)
{
    if (rect.isEmpty()) return;
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
                              const double h)
{
    paintEllipse(QRectF(x, y, w, h));
}

void KisPainter::paintAt(const KisPaintInformation& pi)
{
    if (!d->paintOp) return;
    d->paintOp->paintAt(pi);
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
    for (int i = 0; i < elementCount; i++) {
        QPainterPath::Element element = path.elementAt(i);
        switch (element.type) {
        case QPainterPath::MoveToElement:
            lastPoint =  QPointF(element.x, element.y);
            break;
        case QPainterPath::LineToElement:
            nextPoint =  QPointF(element.x, element.y);
            paintLine(KisPaintInformation(lastPoint), KisPaintInformation(nextPoint));
            lastPoint = nextPoint;
            break;
        case QPainterPath::CurveToElement:
            nextPoint =  QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y);
            paintBezierCurve(KisPaintInformation(lastPoint),
                             QPointF(path.elementAt(i).x, path.elementAt(i).y),
                             QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y),
                             KisPaintInformation(nextPoint));
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

    KisPaintDeviceSP polygon = new KisPaintDevice(d->device->colorSpace());
    Q_CHECK_PTR(polygon);

    if (!d->fillPainter) {
        d->fillPainter = new KisFillPainter(polygon);
    } else {
        d->fillPainter->begin(polygon);
    }


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
        d->fillPainter->fillRect(fillRect, paintColor(), OPACITY_OPAQUE);
        break;
    case FillStyleBackgroundColor:
        d->fillPainter->fillRect(fillRect, backgroundColor(), OPACITY_OPAQUE);
        break;
    case FillStylePattern:
        Q_ASSERT(d->pattern != 0);
        d->fillPainter->fillRect(fillRect, d->pattern);
        break;
    case FillStyleGenerator:
        Q_ASSERT(d->generator != 0);
        d->fillPainter->fillRect(fillRect.x(), fillRect.y(), fillRect.width(), fillRect.height(), generator());
        break;
    }

    if (d->polygonMaskImage.isNull() || (d->maskPainter == 0)) {
        d->polygonMaskImage = QImage(d->maskImageWidth, d->maskImageHeight, QImage::Format_ARGB32);
        d->maskPainter = new QPainter(&d->polygonMaskImage);
        d->maskPainter->setRenderHint(QPainter::Antialiasing, antiAliasPolygonFill());
    }

    // Break the mask up into chunks so we don't have to allocate a potentially very large QImage.
    const QColor opaqueColor(OPACITY_OPAQUE, OPACITY_OPAQUE, OPACITY_OPAQUE, 255);
    const QBrush brush(opaqueColor);
    for (qint32 x = fillRect.x(); x < fillRect.x() + fillRect.width(); x += d->maskImageWidth) {
        for (qint32 y = fillRect.y(); y < fillRect.y() + fillRect.height(); y += d->maskImageHeight) {

            d->polygonMaskImage.fill(qRgba(OPACITY_TRANSPARENT, OPACITY_TRANSPARENT, OPACITY_TRANSPARENT, 255));
            d->maskPainter->translate(-x, -y);
            d->maskPainter->fillPath(path, brush);
            d->maskPainter->translate(x, y);

            qint32 rectWidth = qMin(fillRect.x() + fillRect.width() - x, d->maskImageWidth);
            qint32 rectHeight = qMin(fillRect.y() + fillRect.height() - y, d->maskImageHeight);

            KisHLineIterator lineIt = polygon->createHLineIterator(x, y, rectWidth);

            quint8 tmp;
            for (int row = y; row < y + rectHeight; row++) {
                QRgb* line = reinterpret_cast<QRgb*>(d->polygonMaskImage.scanLine(row - y));
                while (!lineIt.isDone()) {
                    tmp = qRed(line[lineIt.x() - x]);
                    polygon->colorSpace()->applyAlphaU8Mask(lineIt.rawData(),
                                                            &tmp, 1);
                    ++lineIt;
                }
                lineIt.nextRow();
            }

        }
    }

    QRect r = polygon->extent();

    // The strokes for the outline may have already added updated the dirtyrect, but it can't hurt,
    // and if we're painting without outlines, then there will be no dirty rect. Let's do it ourselves...
    bitBlt(r.x(), r.y(), polygon, r.x(), r.y(), r.width(), r.height());
}

void KisPainter::drawLine(const QPointF & start, const QPointF & end)
{
    drawThickLine(start, end, 1, 1);
}

void KisPainter::drawDDALine(const QPointF & start, const QPointF & end)
{
    KisRandomAccessorPixel accessor = d->device->createRandomAccessor(start.x(), start.y(), d->selection);
    int pixelSize = d->device->pixelSize();

    // Width and height of the line
    int xd = (int)(end.x() - start.x());
    int yd = (int)(end.y() - start.y());

    int x = start.x();
    int y = start.y();
    float fx = start.x();
    float fy = start.y();
    float m = (float)yd / (float)xd;
    int y2 = end.y();
    int x2 = end.x();

    if (fabs(m) > 1) {
        int incr;
        if (yd > 0) {
            m = 1.0f / m;
            incr = 1;
        } else {
            m = -1.0f / m;
            incr = -1;
        }
        while (y != y2) {
            fx = fx + m;
            y = y + incr;
            x = (int)(fx + 0.5f);
            accessor.moveTo(x, y);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), d->paintColor.data(), pixelSize);
            }
        }
    } else {
        int incr;
        if (xd > 0) {
            incr = 1;
        } else {
            incr = -1;
            m = -m;
        }
        while (x != x2) {
            fy = fy + m;
            x = x + incr;
            y = (int)(fy + 0.5f);
            accessor.moveTo(x, y);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), d->paintColor.data(), pixelSize);
            }
        }
    }

}

void KisPainter::drawWobblyLine(const QPointF & start, const QPointF & end)
{
    KisRandomAccessorPixel accessor = d->device->createRandomAccessor(start.x(), start.y(), d->selection);
    int pixelSize = d->device->pixelSize();
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

    if (fabs(m) > 1) {
        int incr;
        if (yd > 0) {
            m = 1.0f / m;
            incr = 1;
        } else {
            m = -1.0f / m;
            incr = -1;
        }
        while (y != y2) {
            fx = fx + m;
            y = y + incr;

            x = (int)(fx + 0.5f);
            float br1 = int(fx + 1) - fx;
            float br2 = fx - (int)fx;

            accessor.moveTo(x, y);
            if (accessor.isSelected()) {
                mycolor.setOpacity((int)(255*br1));
                memcpy(accessor.rawData(), mycolor.data(), pixelSize);
            }

            accessor.moveTo(x + 1, y);
            if (accessor.isSelected()) {
                mycolor.setOpacity((int)(255*br2));
                memcpy(accessor.rawData(), mycolor.data(), pixelSize);
            }
        }
    } else {
        int incr;
        if (xd > 0) {
            incr = 1;
        } else {
            incr = -1;
            m = -m;
        }
        while (x != x2) {
            fy = fy + m;
            x = x + incr;
            y = (int)(fy + 0.5f);

            float br1 = int(fy + 1) - fy;
            float br2 = fy - (int)fy;

            accessor.moveTo(x, y);
            if (accessor.isSelected()) {
                mycolor.setOpacity((int)(255*br1));
                memcpy(accessor.rawData(), mycolor.data(), pixelSize);
            }

            accessor.moveTo(x, y + 1);
            if (accessor.isSelected()) {
                mycolor.setOpacity((int)(255*br2));
                memcpy(accessor.rawData(), mycolor.data(), pixelSize);
            }
        }
    }

}

void KisPainter::drawWuLine(const QPointF & start, const QPointF & end)
{
    KisRandomAccessorPixel accessor = d->device->createRandomAccessor(start.x(), start.y(), d->selection);
    int pixelSize = d->device->pixelSize();
    KoColor lineColor(d->paintColor);

    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();


    float grad, xd, yd;
    float xgap, ygap, xend, yend, yf, xf;
    float brightness1, brightness2;

    int ix1, ix2, iy1, iy2;
    int c1, c2;
    const float MaxPixelValue = 255.0f;

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
            accessor.moveTo(ix1, iy1);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), lineColor.data(), pixelSize);
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
            accessor.moveTo(ix1, iy1);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), lineColor.data(), pixelSize);
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

        c1 = (int)(brightness1 * MaxPixelValue);
        c2 = (int)(brightness2 * MaxPixelValue);

        accessor.moveTo(ix1, iy1);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c1);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
        }

        accessor.moveTo(ix1, iy1 + 1);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c2);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
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

        c1 = (int)(brightness1 * MaxPixelValue);
        c2 = (int)(brightness2 * MaxPixelValue);

        accessor.moveTo(ix2, iy2);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c1);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
        }

        accessor.moveTo(ix2, iy2 + 1);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c2);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
        }

        // main loop
        for (int x = ix1 + 1; x <= ix2 - 1; x++) {
            brightness1 = invertFrac(yf);
            brightness2 =    frac(yf);
            c1 = (int)(brightness1 * MaxPixelValue);
            c2 = (int)(brightness2 * MaxPixelValue);

            accessor.moveTo(x, int (yf));
            if (accessor.isSelected()) {
                lineColor.setOpacity(c1);
                memcpy(accessor.rawData(), lineColor.data(), pixelSize);
            }

            accessor.moveTo(x, int (yf) + 1);
            if (accessor.isSelected()) {
                lineColor.setOpacity(c2);
                memcpy(accessor.rawData(), lineColor.data(), pixelSize);
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

        c1 = (int)(brightness1 * MaxPixelValue);
        c2 = (int)(brightness2 * MaxPixelValue);

        accessor.moveTo(ix1, iy1);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c1);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
        }

        accessor.moveTo(x1 + 1, y1);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c2);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
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

        c1 = (int)(brightness1 * MaxPixelValue);
        c2 = (int)(brightness2 * MaxPixelValue);

        accessor.moveTo(ix2, iy2);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c1);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
        }

        accessor.moveTo(ix2 + 1, iy2);
        if (accessor.isSelected()) {
            lineColor.setOpacity(c2);
            memcpy(accessor.rawData(), lineColor.data(), pixelSize);
        }

        // main loop
        for (int y = iy1 + 1; y <= iy2 - 1; y++) {
            brightness1 = invertFrac(xf);
            brightness2 =    frac(xf);
            c1 = (int)(brightness1 * MaxPixelValue);
            c2 = (int)(brightness2 * MaxPixelValue);

            accessor.moveTo(int (xf), y);
            if (accessor.isSelected()) {
                lineColor.setOpacity(c1);
                memcpy(accessor.rawData(), lineColor.data(), pixelSize);
            }

            accessor.moveTo(int (xf) + 1, y);
            if (accessor.isSelected()) {
                lineColor.setOpacity(c2);
                memcpy(accessor.rawData(), lineColor.data(), pixelSize);
            }

            xf = xf + grad;
        }
    }//end-of-else

}

void KisPainter::drawThickLine(const QPointF & start, const QPointF & end, int startWidth, int endWidth)
{
    KisRandomAccessorPixel accessor = d->device->createRandomAccessor(start.x(), start.y(), d->selection);
    int pixelSize = d->device->pixelSize();
    KoColorSpace * cs = d->device->colorSpace();

    KoColor c1(d->paintColor);
    KoColor c2(d->paintColor);
    KoColor c3(d->paintColor);
    KoColor col1(c1);
    KoColor col2(c1);

    float grada, gradb, dxa, dxb, dya, dyb, adya, adyb, fraca, fracb,
    xfa, yfa, xfb, yfb, b1a, b2a, b1b, b2b, dx, dy;
    int x, y, ix1, ix2, iy1, iy2;

    KoColor pix;

    int x0a, y0a, x1a, y1a, x0b, y0b, x1b, y1b;
    int tp0, tn0, tp1, tn1;

    int horizontal = 0;
    float opacity = OPACITY_OPAQUE;

    tp0 = startWidth / 2;
    tn0 = startWidth / 2;
    if (startWidth % 2 == 0) // even width startWidth
        tn0--;

    tp1 = endWidth / 2;
    tn1 = endWidth / 2;
    if (endWidth % 2 == 0) // even width endWidth
        tn1--;

    int x0 = start.x();
    int y0 = start.y();
    int x1 = end.x();
    int y1 = end.y();

    dx = x1 - x0; // run of general line
    dy = y1 - y0; // rise of general line

    if (dy < 0) dy = -dy;
    if (dx < 0) dx = -dx;

    if (dx > dy) { // horizontalish
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
            accessor.moveTo(x0, i);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), c1.data(), pixelSize);
            }
        }
        for (int i = y1a; i <= y1b; i++) {
            accessor.moveTo(x1, i);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), c1.data(), pixelSize);
            }
        }

    } else {
        for (int i = x0a; i <= x0b; i++) {
            accessor.moveTo(i, y0);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), c1.data(), pixelSize);
            }
        }
        for (int i = x1a; i <= x1b; i++) {
            accessor.moveTo(i, y1);
            if (accessor.isSelected()) {
                memcpy(accessor.rawData(), c1.data(), pixelSize);
            }
        }
    }

    //antialias endpoints
    if (x1 != x0 && y1 != y0) {
        if (horizontal) {
            accessor.moveTo(x0a, y0a - 1);
            if (accessor.isSelected()) {
                quint8 alpha = cs->alpha(accessor.rawData());
                opacity = .25 * c1.opacity() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                memcpy(accessor.rawData(), col1.data(), pixelSize);
            }

            accessor.moveTo(x1b, y1b + 1);
            if (accessor.isSelected()) {
                quint8 alpha = cs->alpha(accessor.rawData());
                opacity = .25 * c2.opacity() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                memcpy(accessor.rawData(), col1.data(), pixelSize);
            }

        } else {
            accessor.moveTo(x0a - 1, y0a);
            if (accessor.isSelected()) {
                quint8 alpha = cs->alpha(accessor.rawData());
                opacity = .25 * c1.opacity() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                memcpy(accessor.rawData(), col1.data(), pixelSize);
            }

            accessor.moveTo(x1b + 1, y1b);
            if (accessor.isSelected()) {
                quint8 alpha = cs->alpha(accessor.rawData());
                opacity = .25 * c2.opacity() + (1 - .25) * alpha;
                col1.setOpacity(opacity);
                memcpy(accessor.rawData(), col1.data(), pixelSize);
            }
        }
    }

    dxa = x1a - x0a; // run of a
    dya = y1a - y0a; // rise of a
    dxb = x1b - x0b; // run of b
    dyb = y1b - y0b; // rise of b

    if (dya < 0) adya = -dya;
    else adya = dya;
    if (dyb < 0) adyb = -dyb;
    else adyb = dyb;


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
            opacity = ((x - ix1) / dx) * c2.opacity() + (1 - (x - ix1) / dx) * c1.opacity();
            c3.setOpacity(static_cast<int>(opacity));

            accessor.moveTo(x, (int)yfa);
            if (accessor.isSelected()) {
                quint8 alpha = cs->alpha(accessor.rawData());
                opacity = b1a * c3.opacity() + (1 - b1a) * alpha;
                col1.setOpacity(opacity);
                memcpy(accessor.rawData(), col1.data(), pixelSize);
            }

            // color first pixel of top line
            if (!(startWidth == 1 && endWidth == 1)) {
                accessor.moveTo(x, (int)yfb);
                if (accessor.isSelected()) {
                    quint8 alpha = cs->alpha(accessor.rawData());
                    opacity = b1b * c3.opacity() + (1 - b1b) * alpha;
                    col1.setOpacity(opacity);
                    memcpy(accessor.rawData(), col1.data(), pixelSize);
                }
            }

            // color second pixel of bottom line
            if (grada != 0 && grada != 1) { // if not flat or exact diagonal
                accessor.moveTo(x, int (yfa) + 1);
                if (accessor.isSelected()) {
                    quint8 alpha = cs->alpha(accessor.rawData());
                    opacity = b2a * c3.opacity() + (1 - b2a)  * alpha;
                    col2.setOpacity(opacity);
                    memcpy(accessor.rawData(), col2.data(), pixelSize);
                }

            }

            // color second pixel of top line
            if (gradb != 0 && gradb != 1 && !(startWidth == 1 && endWidth == 1)) {
                accessor.moveTo(x, int (yfb) + 1);
                if (accessor.isSelected()) {
                    quint8 alpha = cs->alpha(accessor.rawData());
                    opacity = b2b * c3.opacity() + (1 - b2b) * alpha;
                    col2.setOpacity(opacity);
                    memcpy(accessor.rawData(), col2.data(), pixelSize);
                }

            }

            // fill remaining pixels
            if (!(startWidth == 1 && endWidth == 1)) {
                if (yfa < yfb)
                    for (int i = yfa + 1; i <= yfb; i++) {
                        accessor.moveTo(x, i);
                        if (accessor.isSelected()) {
                            memcpy(accessor.rawData(), c3.data(), pixelSize);
                        }
                    }
                else
                    for (int i = yfa + 1; i >= yfb; i--) {
                        accessor.moveTo(x, i);
                        if (accessor.isSelected()) {
                            memcpy(accessor.rawData(), c3.data(), pixelSize);
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
            opacity = ((y - iy1) / dy) * c2.opacity() + (1 - (y - iy1) / dy) * c1.opacity();
            c3.setOpacity(static_cast<int>(opacity));

            accessor.moveTo(int (xfa), y);
            if (accessor.isSelected()) {
                quint8 alpha = cs->alpha(accessor.rawData());
                opacity = b1a * c3.opacity() + (1 - b1a) * alpha;
                col1.setOpacity(opacity);
                memcpy(accessor.rawData(), col1.data(), pixelSize);
            }

            // color first pixel of right line
            if (!(startWidth == 1 && endWidth == 1)) {
                accessor.moveTo(int(xfb), y);
                if (accessor.isSelected()) {
                    quint8 alpha = cs->alpha(accessor.rawData());
                    opacity = b1b * c3.opacity() + (1 - b1b)  * alpha;
                    col1.setOpacity(opacity);
                    memcpy(accessor.rawData(), col1.data(), pixelSize);
                }
            }

            // color second pixel of left line
            if (grada != 0 && grada != 1) { // if not flat or exact diagonal
                accessor.moveTo(int(xfa) + 1, y);
                if (accessor.isSelected()) {
                    quint8 alpha = cs->alpha(accessor.rawData());
                    opacity = b2a * c3.opacity() + (1 - b2a) * alpha;
                    col2.setOpacity(opacity);
                    memcpy(accessor.rawData(), col2.data(), pixelSize);
                }

            }

            // color second pixel of right line
            if (gradb != 0 && gradb != 1 && !(startWidth == 1 && endWidth == 1)) {
                accessor.moveTo(int(xfb) + 1, y);
                if (accessor.isSelected()) {
                    quint8 alpha = cs->alpha(accessor.rawData());
                    opacity = b2b * c3.opacity() + (1 - b2b) * alpha;
                    col2.setOpacity(opacity);
                    memcpy(accessor.rawData(), col2.data(), pixelSize);
                }
            }

            // fill remaining pixels between current xfa,xfb
            if (!(startWidth == 1 && endWidth == 1)) {
                if (xfa < xfb)
                    for (int i = (int) xfa + 1; i <= (int) xfb; i++) {
                        accessor.moveTo(i, y);
                        if (accessor.isSelected()) {
                            memcpy(accessor.rawData(), c3.data(), pixelSize);
                        }
                    }
                else
                    for (int i = (int) xfb; i <= (int) xfa + 1; i++) {
                        accessor.moveTo(i, y);
                        if (accessor.isSelected()) {
                            memcpy(accessor.rawData(), c3.data(), pixelSize);
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

KisTransaction  * KisPainter::transaction()
{
    return d->transaction;
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
    d->channelFlags = channelFlags;
}

QBitArray KisPainter::channelFlags()
{
    return d->channelFlags;
}

void KisPainter::setPattern(KisPattern * pattern)
{
    d->pattern = pattern;
}

KisPattern * KisPainter::pattern() const
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

void KisPainter::setGenerator(KisFilterConfiguration * generator)
{
    d->generator = generator;
}

KisFilterConfiguration * KisPainter::generator() const
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

void KisPainter::setOpacity(quint8 opacity)
{
    d->opacity = opacity;
}

quint8 KisPainter::opacity() const
{
    return d->opacity;
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

const KoAbstractGradient* KisPainter::gradient()
{
    return d->gradient;
}

void KisPainter::setPaintOpPreset(KisPaintOpPresetSP preset, KisImageWSP image)
{
    d->paintOpPreset = preset;
    delete d->paintOp;
    d->paintOp = KisPaintOpRegistry::instance()->paintOp(preset, this, image);
}

KisPaintOpPresetSP KisPainter::preset() const
{
    return d->paintOpPreset;
}

KisPaintOp* KisPainter::paintOp() const
{
    return d->paintOp;
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
    d->alphaLocked = protect;
}

bool KisPainter::alphaLocked() const
{
    return d->alphaLocked;
}

