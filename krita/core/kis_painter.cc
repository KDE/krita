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

#include "qbrush.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qregion.h"
#include "qwmatrix.h"
#include <qimage.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qstring.h>

#include <kdebug.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_colorspace.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_color.h"

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
    m_compositeOp = COMPOSITE_OVER;
    m_dab = 0;
    m_fillStyle = FillStyleNone;
    m_strokeStyle = StrokeStyleBrush;
    m_pressure = PRESSURE_MIN;
    m_duplicateHealing = false;
    m_duplicateHealingRadius = 10;
    m_duplicatePerspectiveCorrection = false;
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
    m_pixelSize = device->pixelSize();
}

KCommand *KisPainter::end()
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


KCommand *KisPainter::endTransaction()
{
    KCommand *command = m_transaction;
        m_transaction = 0;
        return command;
}


QRect KisPainter::dirtyRect() {
    QRect r = m_dirtyRect;
    m_dirtyRect = QRect();
    return r;
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy,
                        const KisCompositeOp& op,
                        KisPaintDeviceSP srcdev,
                        Q_UINT8 opacity,
                        Q_INT32 sx, Q_INT32 sy,
                        Q_INT32 sw, Q_INT32 sh)
{
    if (srcdev == 0) {
        return;
    }

    QRect srcRect = QRect(sx, sy, sw, sh);

    if (srcdev->extentIsValid() && op != COMPOSITE_COPY) {
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

    KisColorSpace * srcCs = srcdev->colorSpace();

    Q_INT32 dstY = dy;
    Q_INT32 srcY = sy;
    Q_INT32 rowsRemaining = sh;

    while (rowsRemaining > 0) {

        Q_INT32 dstX = dx;
        Q_INT32 srcX = sx;
        Q_INT32 columnsRemaining = sw;
        Q_INT32 numContiguousDstRows = m_device->numContiguousRows(dstY, dstX, dstX + sw - 1);
        Q_INT32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);

        Q_INT32 rows = QMIN(numContiguousDstRows, numContiguousSrcRows);
        rows = QMIN(rows, rowsRemaining);

        while (columnsRemaining > 0) {

            Q_INT32 numContiguousDstColumns = m_device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
            Q_INT32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);

            Q_INT32 columns = QMIN(numContiguousDstColumns, numContiguousSrcColumns);
            columns = QMIN(columns, columnsRemaining);

            Q_INT32 srcRowStride = srcdev->rowStride(srcX, srcY);
            //const Q_UINT8 *srcData = srcdev->pixel(srcX, srcY);
            KisHLineIteratorPixel srcIt = srcdev->createHLineIterator(srcX, srcY, columns, false);
            const Q_UINT8 *srcData = srcIt.rawData();

            //Q_UINT8 *dstData = m_device->writablePixel(dstX, dstY);
            Q_INT32 dstRowStride = m_device->rowStride(dstX, dstY);
            KisHLineIteratorPixel dstIt = m_device->createHLineIterator(dstX, dstY, columns, true);
            Q_UINT8 *dstData = dstIt.rawData();


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
                          op);

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }
}

void KisPainter::bltSelection(Q_INT32 dx, Q_INT32 dy,
                  const KisCompositeOp &op,
                  KisPaintDeviceSP srcdev,
                  KisSelectionSP seldev,
                  Q_UINT8 opacity,
                  Q_INT32 sx, Q_INT32 sy,
                  Q_INT32 sw, Q_INT32 sh)
{
    // Better use a probablistic method than a too slow one
    if (seldev->isProbablyTotallyUnselected(QRect(dx, dy, sw, sh))) {
/*
        kdDebug() << "Blitting outside selection rect\n";

        kdDebug() << "srcdev: " << srcdev << " (" << srcdev->name() << ")"
        << ", seldev: " << seldev << " (" << seldev->name() << ")"
        << ". dx, dy " << dx << "," << dy
        << ". sx, sy : sw, sy " << sx << "," << sy << " : " << sw << "," << sh << endl;
*/
        return;
    }
    bltMask(dx,dy,op,srcdev,seldev.data(),opacity,sx,sy,sw,sh);
}

void KisPainter::bltMask(Q_INT32 dx, Q_INT32 dy,
                     const KisCompositeOp &op,
                     KisPaintDeviceSP srcdev,
                     KisPaintDeviceSP seldev,
                     Q_UINT8 opacity,
                     Q_INT32 sx, Q_INT32 sy,
                     Q_INT32 sw, Q_INT32 sh)

{
    if (srcdev == 0) return;

    if (seldev == 0) return;

    if (m_device == 0) return;


    QRect srcRect = QRect(sx, sy, sw, sh);

    if (srcdev->extentIsValid() && op != COMPOSITE_COPY) {
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

    KisColorSpace * srcCs = srcdev->colorSpace();

    Q_INT32 dstY = dy;
    Q_INT32 srcY = sy;
    Q_INT32 rowsRemaining = sh;

    while (rowsRemaining > 0) {

        Q_INT32 dstX = dx;
        Q_INT32 srcX = sx;
        Q_INT32 columnsRemaining = sw;
        Q_INT32 numContiguousDstRows = m_device->numContiguousRows(dstY, dstX, dstX + sw - 1);
        Q_INT32 numContiguousSrcRows = srcdev->numContiguousRows(srcY, srcX, srcX + sw - 1);
        Q_INT32 numContiguousSelRows = seldev->numContiguousRows(dstY, dstX, dstX + sw - 1);

        Q_INT32 rows = QMIN(numContiguousDstRows, numContiguousSrcRows);
        rows = QMIN(numContiguousSelRows, rows);
        rows = QMIN(rows, rowsRemaining);

        while (columnsRemaining > 0) {

            Q_INT32 numContiguousDstColumns = m_device->numContiguousColumns(dstX, dstY, dstY + rows - 1);
            Q_INT32 numContiguousSrcColumns = srcdev->numContiguousColumns(srcX, srcY, srcY + rows - 1);
            Q_INT32 numContiguousSelColumns = seldev->numContiguousColumns(dstX, dstY, dstY + rows - 1);

            Q_INT32 columns = QMIN(numContiguousDstColumns, numContiguousSrcColumns);
            columns = QMIN(numContiguousSelColumns, columns);
            columns = QMIN(columns, columnsRemaining);

            //Q_UINT8 *dstData = m_device->writablePixel(dstX, dstY);
            Q_INT32 dstRowStride = m_device->rowStride(dstX, dstY);
            KisHLineIteratorPixel dstIt = m_device->createHLineIterator(dstX, dstY, columns, true);
            Q_UINT8 *dstData = dstIt.rawData();

            //const Q_UINT8 *srcData = srcdev->pixel(srcX, srcY);
            Q_INT32 srcRowStride = srcdev->rowStride(srcX, srcY);
            KisHLineIteratorPixel srcIt = srcdev->createHLineIterator(srcX, srcY, columns, false);
            const Q_UINT8 *srcData = srcIt.rawData();

            //const Q_UINT8 *selData = seldev->pixel(dstX, dstY);
            Q_INT32 selRowStride = seldev->rowStride(dstX, dstY);
            KisHLineIteratorPixel selIt = seldev->createHLineIterator(dstX, dstY, columns, false);
            const Q_UINT8 *selData = selIt.rawData();

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
                                   op);

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }
}


void KisPainter::bltSelection(Q_INT32 dx, Q_INT32 dy,
                  const KisCompositeOp& op,
                  KisPaintDeviceSP srcdev,
                  Q_UINT8 opacity,
                  Q_INT32 sx, Q_INT32 sy,
                  Q_INT32 sw, Q_INT32 sh)
{
    if (m_device == 0) return;
    if (!m_device->hasSelection()) {
        bitBlt(dx, dy, op, srcdev, opacity, sx, sy, sw, sh);
    }
    else
        bltSelection(dx,dy,op,srcdev, m_device->selection(),opacity,sx,sy,sw,sh);
}

double KisPainter::paintLine(const KisPoint & pos1,
                 const double pressure1,
                 const double xTilt1,
                 const double yTilt1,
                 const KisPoint & pos2,
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

        KisPoint p(start.x() + (step.x() / xScale), start.y() + (step.y() / yScale));

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

    if (dist > 0)
        return dist;
    else
        return 0;
}

void KisPainter::paintPolyline (const vKisPoint &points,
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

void KisPainter::getBezierCurvePoints(const KisPoint &pos1,
                      const KisPoint &control1,
                      const KisPoint &control2,
                      const KisPoint &pos2,
                      vKisPoint& points)
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

        getBezierCurvePoints(l1.toKisPoint(), l2.toKisPoint(), l3.toKisPoint(), l4.toKisPoint(), points);
        getBezierCurvePoints(r1.toKisPoint(), r2.toKisPoint(), r3.toKisPoint(), r4.toKisPoint(), points);
    }
}

double KisPainter::paintBezierCurve(const KisPoint &pos1,
                    const double pressure1,
                    const double xTilt1,
                    const double yTilt1,
                    const KisPoint &control1,
                    const KisPoint &control2,
                    const KisPoint &pos2,
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

        newDistance = paintBezierCurve(l1.toKisPoint(), pressure1, xTilt1, yTilt1,
                           l2.toKisPoint(), l3.toKisPoint(),
                           l4.toKisPoint(), midPressure, midXTilt, midYTilt,
                           savedDist);
        newDistance = paintBezierCurve(r1.toKisPoint(), midPressure, midXTilt, midYTilt,
                           r2.toKisPoint(),
                           r3.toKisPoint(),
                           r4.toKisPoint(), pressure2, xTilt2, yTilt2, newDistance);
    }

    return newDistance;
}

void KisPainter::paintRect (const KisPoint &startPoint,
                            const KisPoint &endPoint,
                            const double /*pressure*/,
                const double /*xTilt*/,
                const double /*yTilt*/)
{
    KoRect normalizedRect = KisRect (startPoint, endPoint).normalize ();

    vKisPoint points;

    points.push_back(normalizedRect.topLeft());
    points.push_back(normalizedRect.bottomLeft());
    points.push_back(normalizedRect.bottomRight());
    points.push_back(normalizedRect.topRight());

    paintPolygon(points);
}

void KisPainter::paintEllipse (const KisPoint &startPoint,
                               const KisPoint &endPoint,
                               const double /*pressure*/,
                   const double /*xTilt*/,
                   const double /*yTilt*/)
{
    KisRect r = KisRect(startPoint, endPoint).normalize();

    // See http://www.whizkidtech.redprince.net/bezier/circle/ for explanation.
    // kappa = (4/3*(sqrt(2)-1))
    const double kappa = 0.5522847498;
    const double lx = (r.width() / 2) * kappa;
    const double ly = (r.height() / 2) * kappa;

    KisPoint center = r.center();

    KisPoint p0(r.left(), center.y());
    KisPoint p1(r.left(), center.y() - ly);
    KisPoint p2(center.x() - lx, r.top());
    KisPoint p3(center.x(), r.top());

    vKisPoint points;

    getBezierCurvePoints(p0, p1, p2, p3, points);

    KisPoint p4(center.x() + lx, r.top());
    KisPoint p5(r.right(), center.y() - ly);
    KisPoint p6(r.right(), center.y());

    getBezierCurvePoints(p3, p4, p5, p6, points);

    KisPoint p7(r.right(), center.y() + ly);
    KisPoint p8(center.x() + lx, r.bottom());
    KisPoint p9(center.x(), r.bottom());

    getBezierCurvePoints(p6, p7, p8, p9, points);

    KisPoint p10(center.x() - lx, r.bottom());
    KisPoint p11(r.left(), center.y() + ly);

    getBezierCurvePoints(p9, p10, p11, p0, points);

    paintPolygon(points);
}

void KisPainter::paintAt(const KisPoint & pos,
                         const double pressure,
                         const double xTilt,
                         const double yTilt)
{
    if (!m_paintOp) return;
    m_paintOp->paintAt(pos, KisPaintInformation(pressure, xTilt, yTilt, KisVector2D()));
}

double KisPainter::pointToLineDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1)
{
    double lineLength = sqrt((l1.x() - l0.x()) * (l1.x() - l0.x()) + (l1.y() - l0.y()) * (l1.y() - l0.y()));
    double distance = 0;

    if (lineLength > DBL_EPSILON) {
        distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / lineLength;
        distance = fabs(distance);
    }

    return distance;
}

/*
 * Concave Polygon Scan Conversion
 * by Paul Heckbert
 * from "Graphics Gems", Academic Press, 1990
 */

/*
 * concave: scan convert nvert-sided concave non-simple polygon with vertices at
 * (point[i].x, point[i].y) for i in [0..nvert-1] within the window win by
 * calling spanproc for each visible span of pixels.
 * Polygon can be clockwise or counterclockwise.
 * Algorithm does uniform point sampling at pixel centers.
 * Inside-outside test done by Jordan's rule: a point is considered inside if
 * an emanating ray intersects the polygon an odd number of times.
 * drawproc should fill in pixels from xl to xr inclusive on scanline y,
 * e.g:
 *    drawproc(y, xl, xr)
 *    int y, xl, xr;
 *    {
 *        int x;
 *        for (x=xl; x<=xr; x++)
 *        pixel_write(x, y, pixelvalue);
 *    }
 *
 *  Paul Heckbert    30 June 81, 18 Dec 89
 */

typedef struct {    /* a polygon edge */
    double x;       /* x coordinate of edge's intersection with current scanline */
    double dx;      /* change in x with respect to y */
    int i;            /* edge number: edge i goes from pt[i] to pt[i+1] */
} Edge;

static int n;            /* number of vertices */
static const KisPoint *pt;    /* vertices */

static int nact;        /* number of active edges */
static Edge *active;        /* active edge list:edges crossing scanline y */

/* comparison routines for qsort */
static int compare_ind(const void *pu, const void *pv)
{
    const int *u = static_cast<const int *>(pu);
    const int *v = static_cast<const int *>(pv);

    return pt[*u].y() <= pt[*v].y() ? -1 : 1;
}

static int compare_active(const void *pu, const void *pv)
{
    const Edge *u = static_cast<const Edge *>(pu);
    const Edge *v = static_cast<const Edge *>(pv);

    return u->x <= v->x ? -1 : 1;
}

static void cdelete(int i)        /* remove edge i from active list */
{
    int j;

    for (j=0; j<nact && active[j].i!=i; j++);
    if (j>=nact) return;        /* edge not in active list; happens at win->y0*/
    nact--;
    bcopy(&active[j+1], &active[j], (nact-j)*sizeof active[0]);
}

static void cinsert(int i, int y)        /* append edge i to end of active list */
{
    int j;
    double dx;
    const KisPoint *p, *q;

    j = i<n-1 ? i+1 : 0;
    if (pt[i].y() < pt[j].y()) {
        p = &pt[i]; q = &pt[j];
    } else {
        p = &pt[j]; q = &pt[i];
    }
    /* initialize x position at intersection of edge with scanline y */
    active[nact].dx = dx = (q->x()-p->x())/(q->y()-p->y());
    active[nact].x = dx*(y+.5-p->y())+p->x();
    active[nact].i = i;
    nact++;
}

void KisPainter::fillPolygon(const vKisPoint& points, FillStyle fillStyle)
{
    int nvert = points.count();
    int k, y0, y1, y, i, j, xl, xr;
    int *ind;        /* list of vertex indices, sorted by pt[ind[j]].y */

    n = nvert;
    pt = &(points[0]);
    if (n<3) return;
    if (fillStyle == FillStyleNone) {
        return;
    }

    ind = new int[n];
    Q_CHECK_PTR(ind);
    active = new Edge[n];
    Q_CHECK_PTR(active);

    /* create y-sorted array of indices ind[k] into vertex list */
    for (k=0; k<n; k++)
        ind[k] = k;
    qsort(ind, n, sizeof ind[0], compare_ind);  /* sort ind by pt[ind[k]].y */

    nact = 0;                /* start with empty active list */
    k = 0;                    /* ind[k] is next vertex to process */
    y0 = static_cast<int>(ceil(pt[ind[0]].y()-.5));            /* ymin of polygon */
    y1 = static_cast<int>(floor(pt[ind[n-1]].y()-.5));        /* ymax of polygon */

    int x0 = INT_MAX;
    int x1 = INT_MIN;

    for (int i = 0; i < nvert; i++) {
        int pointHighX = static_cast<int>(ceil(points[i].x() - 0.5));
        int pointLowX = static_cast<int>(floor(points[i].x() - 0.5));

        if (pointLowX < x0) {
            x0 = pointLowX;
        }
        if (pointHighX > x1) {
            x1 = pointHighX;
        }
    }

    // Fill the polygon bounding rectangle with the required contents then we'll
    // create a mask for the actual polygon coverage.

    KisPaintDeviceSP polygon = new KisPaintDevice(m_device->colorSpace(), "polygon");
    Q_CHECK_PTR(polygon);

    KisFillPainter fillPainter(polygon);
    QRect boundingRectangle(x0, y0, x1 - x0 + 1, y1 - y0 + 1);

    // Clip to the image bounds.
    if (m_device->image()) {
        boundingRectangle &= m_device->image()->bounds();
    }

    switch (fillStyle) {
    default:
        // Fall through
    case FillStyleGradient:
        // Currently unsupported, fall through
    case FillStyleStrokes:
        // Currently unsupported, fall through
        kdWarning(DBG_AREA_CORE) << "Unknown or unsupported fill style in fillPolygon\n";
    case FillStyleForegroundColor:
        fillPainter.fillRect(boundingRectangle, paintColor(), OPACITY_OPAQUE);
        break;
    case FillStyleBackgroundColor:
        fillPainter.fillRect(boundingRectangle, backgroundColor(), OPACITY_OPAQUE);
        break;
    case FillStylePattern:
        Q_ASSERT(m_pattern != 0);
        fillPainter.fillRect(boundingRectangle, m_pattern);
        break;
    }

    KisSelectionSP polygonMask = new KisSelection(polygon);

    for (y=y0; y<=y1; y++) {        /* step through scanlines */
        /* scanline y is at y+.5 in continuous coordinates */

        /* check vertices between previous scanline and current one, if any */
        for (; k<n && pt[ind[k]].y()<=y+.5; k++) {
            /* to simplify, if pt.y=y+.5, pretend it's above */
            /* invariant: y-.5 < pt[i].y <= y+.5 */
            i = ind[k];
            /*
             * insert or delete edges before and after vertex i (i-1 to i,
             * and i to i+1) from active list if they cross scanline y
             */
            j = i>0 ? i-1 : n-1;        /* vertex previous to i */
            if (pt[j].y() <= y-.5)        /* old edge, remove from active list */
                cdelete(j);
            else if (pt[j].y() > y+.5)    /* new edge, add to active list */
                cinsert(j, y);
            j = i<n-1 ? i+1 : 0;        /* vertex next after i */
            if (pt[j].y() <= y-.5)        /* old edge, remove from active list */
                cdelete(i);
            else if (pt[j].y() > y+.5)    /* new edge, add to active list */
                cinsert(i, y);
        }

        /* sort active edge list by active[j].x */
        qsort(active, nact, sizeof active[0], compare_active);

        /* draw horizontal segments for scanline y */
        for (j=0; j<nact; j+=2) {    /* draw horizontal segments */
            /* span 'tween j & j+1 is inside, span tween j+1 & j+2 is outside */
            xl = static_cast<int>(ceil(active[j].x-.5));        /* left end of span */
            xr = static_cast<int>(floor(active[j+1].x-.5));        /* right end of span */

            if (xl<=xr) {
                KisHLineIterator it = polygonMask->createHLineIterator(xl, y, xr - xl + 1, true);

                while (!it.isDone()) {
                    // We're using a selection here, that means alpha colorspace, that means one byte.
                    it.rawData()[0] = MAX_SELECTED;
                    ++it;
                }
            }

            active[j].x += active[j].dx;        /* increment edge coords */
            active[j+1].x += active[j+1].dx;
        }
    }
    delete [] ind;
    delete [] active;

    polygon->applySelectionMask(polygonMask);

    QRect r = polygon->extent();

    // The strokes for the outline may have already added updated the dirtyrect, but it can't hurt,
    // and if we're painting without outlines, then there will be no dirty rect. Let's do it ourselves...
    // addDirtyRect( r ); // XXX the bltSelection will add to the dirtyrect

    bltSelection(r.x(), r.y(), compositeOp(), polygon, opacity(), r.x(), r.y(), r.width(), r.height());
}

void KisPainter::paintPolygon(const vKisPoint& points)
{
    if (m_fillStyle != FillStyleNone) {
        fillPolygon(points, m_fillStyle);
    }

    if (m_strokeStyle != StrokeStyleNone) {
        if (points.count() > 1) {
            double distance = -1;

            for (uint i = 0; i < points.count() - 1; i++) {
                distance = paintLine(points[i], PRESSURE_DEFAULT, 0, 0, points[i + 1], PRESSURE_DEFAULT, 0, 0, distance);
            }
            paintLine(points[points.count() - 1], PRESSURE_DEFAULT, 0, 0, points[0], PRESSURE_DEFAULT, 0, 0, distance);
        }
    }
}

