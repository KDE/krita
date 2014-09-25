/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_cage_transform_worker.h"

#include "kis_grid_interpolation_tools.h"
#include "kis_green_coordinates_math.h"
#include "kis_algebra_2d.h"

#include <QPainter>

#include "KoColor.h"
#include "kis_selection.h"
#include "kis_painter.h"

#include "kis_four_point_interpolator_forward.h"


struct KisCageTransformWorker::Private
{
    Private(KisPaintDeviceSP _dev,
            const QVector<QPointF> &_origCage,
            KoUpdater *_progress,
            int _pixelPrecision)
        : dev(_dev),
          origCage(_origCage),
          progress(_progress),
          pixelPrecision(_pixelPrecision)
    {
    }

    KisPaintDeviceSP dev;

    QImage srcImage;
    QPointF srcImageOffset;

    QVector<QPointF> origCage;
    QVector<QPointF> transfCage;
    KoUpdater *progress;
    int pixelPrecision;

    QVector<int> allToValidPointsMap;
    QVector<QPointF> validPoints;

    /**
     * Contains all points fo the grid including non-defined
     * points (the ones which are placed outside the cage).
     */
    QVector<QPointF> allSrcPoints;

    KisGreenCoordinatesMath cage;

    QSize gridSize;

    QVector<QPointF> calculateTransformedPoints();

    /**
     *    A-----B         The polygons will be in the following order:
     *    |     |
     *    |     |         polygon << A << B << D << C;
     *    C-----D
     */
    inline QVector<int> calculateCellIndexes(int col, int row);

    inline QVector<int> calculateMappedIndexes(int col, int row,
                                               int *numExistingPoints);

    inline int pointToIndex(const QPoint &cellPt);

    int tryGetValidIndex(const QPoint &cellPt);

    bool getOrthogonalPointApproximation(const QPoint &cellPt,
                                         const QVector<QPointF> &transformedPoints,
                                         QPointF *srcPoint,
                                         QPointF *dstPoint);

    inline QPoint pointIndexToColRow(QPoint baseColRow, int index);

    template <class PolygonOp>
    void iterateThroughGrid(PolygonOp polygonOp,
                       const QVector<QPointF> &transformedPoints);
};

KisCageTransformWorker::KisCageTransformWorker(KisPaintDeviceSP dev,
                                               const QVector<QPointF> &origCage,
                                               KoUpdater *progress,
                                               int pixelPrecision)
    : m_d(new Private(dev, origCage, progress, pixelPrecision))
{
}

KisCageTransformWorker::KisCageTransformWorker(const QImage &srcImage,
                                               const QPointF &srcImageOffset,
                                               const QVector<QPointF> &origCage,
                                               KoUpdater *progress,
                                               int pixelPrecision)
    : m_d(new Private(0, origCage, progress, pixelPrecision))
{
    m_d->srcImage = srcImage;
    m_d->srcImageOffset = srcImageOffset;
}

KisCageTransformWorker::~KisCageTransformWorker()
{
}

void KisCageTransformWorker::setTransformedCage(const QVector<QPointF> &transformedCage)
{
    m_d->transfCage = transformedCage;
}

struct PointsFetcherOp
{
    static const QPointF invalidPoint;
    static inline bool isPointValid(const QPointF &pt) {
        return pt.x() > -1e10 && pt.y() > -1e10;
    }


    PointsFetcherOp(const QPolygonF &cagePolygon)
        : m_cagePolygon(cagePolygon),
          m_numValidPoints(0)
    {
        m_polygonDirection = KisAlgebra2D::polygonDirection(cagePolygon);
    }

    inline void processPoint(int col, int row,
                             int prevCol, int prevRow,
                             int colIndex, int rowIndex) {

        Q_UNUSED(prevCol);
        Q_UNUSED(prevRow);
        Q_UNUSED(colIndex);
        Q_UNUSED(rowIndex);

        QPointF pt(col, row);

        if (m_cagePolygon.containsPoint(pt, Qt::OddEvenFill)) {
            KisAlgebra2D::adjustIfOnPolygonBoundary(m_cagePolygon, m_polygonDirection, &pt);

            m_points << pt;
            m_pointValid << true;
            m_numValidPoints++;
        } else {
            m_points << pt;
            m_pointValid << false;
        }
    }

    inline void nextLine() {
    }

    QVector<bool> m_pointValid;
    QVector<QPointF> m_points;
    QPolygonF m_cagePolygon;
    int m_polygonDirection;
    int m_numValidPoints;
};

const QPointF PointsFetcherOp::invalidPoint(-1e12, -1e12);

void KisCageTransformWorker::prepareTransform()
{
    if (m_d->origCage.size() < 3) return;

    const QPolygonF srcPolygon(m_d->origCage);

    QRect srcBounds = m_d->dev ? m_d->dev->region().boundingRect() :
        QRectF(m_d->srcImageOffset, m_d->srcImage.size()).toAlignedRect();
    srcBounds &= srcPolygon.boundingRect().toAlignedRect();

    m_d->gridSize =
        GridIterationTools::calcGridSize(srcBounds, m_d->pixelPrecision);

    PointsFetcherOp pointsOp(srcPolygon);
    GridIterationTools::processGrid(pointsOp, srcBounds, m_d->pixelPrecision);

    const int numPoints = pointsOp.m_points.size();

    KIS_ASSERT_RECOVER_RETURN(numPoints == m_d->gridSize.width() * m_d->gridSize.height());

    m_d->allSrcPoints = pointsOp.m_points;
    m_d->allToValidPointsMap.resize(pointsOp.m_points.size());
    m_d->validPoints.resize(pointsOp.m_numValidPoints);

    {
        int validIdx = 0;
        for (int i = 0; i < numPoints; i++) {
            const QPointF &pt = pointsOp.m_points[i];
            const bool pointValid = pointsOp.m_pointValid[i];

            if (pointValid) {
                m_d->validPoints[validIdx] = pt;
                m_d->allToValidPointsMap[i] = validIdx;
                validIdx++;
            } else {
                m_d->allToValidPointsMap[i] = -1;
            }
        }
        KIS_ASSERT_RECOVER_NOOP(validIdx == m_d->validPoints.size());
    }

    m_d->cage.precalculateGreenCoordinates(m_d->origCage, m_d->validPoints);
}

QVector<QPointF> KisCageTransformWorker::Private::calculateTransformedPoints()
{
    cage.generateTransformedCageNormals(transfCage);

    const int numValidPoints = validPoints.size();
    QVector<QPointF> transformedPoints(numValidPoints);

    for (int i = 0; i < numValidPoints; i++) {
        transformedPoints[i] = cage.transformedPoint(i, transfCage);
    }

    return transformedPoints;
}

inline QVector<int> KisCageTransformWorker::Private::
calculateCellIndexes(int col, int row)
{
    const int tl = col + row * gridSize.width();
    const int tr = tl + 1;
    const int bl = tl + gridSize.width();
    const int br = bl + 1;

    QVector<int> cellIndexes;
    cellIndexes << tl;
    cellIndexes << tr;
    cellIndexes << br;
    cellIndexes << bl;

    return cellIndexes;
}

inline QVector<int> KisCageTransformWorker::Private::
calculateMappedIndexes(int col, int row,
                       int *numExistingPoints)
{
    *numExistingPoints = 0;
    QVector<int> cellIndexes = calculateCellIndexes(col, row);

    for (int i = 0; i < 4; i++) {
        cellIndexes[i] = allToValidPointsMap[cellIndexes[i]];
        *numExistingPoints += cellIndexes[i] >= 0;
    }

    return cellIndexes;
}

inline int KisCageTransformWorker::Private::
pointToIndex(const QPoint &cellPt)
{
    return cellPt.x() +
        cellPt.y() * gridSize.width();
}

int KisCageTransformWorker::Private::
tryGetValidIndex(const QPoint &cellPt)
{
    int index = -1;

    return
        cellPt.x() >= 0 &&
        cellPt.y() >= 0 &&
        cellPt.x() < gridSize.width() - 1 &&
        cellPt.y() < gridSize.height() - 1 &&
        (index = allToValidPointsMap[pointToIndex(cellPt)]) >= 0, index;
}

struct PointExtension {
    int near;
    int far;
};

bool KisCageTransformWorker::Private::
getOrthogonalPointApproximation(const QPoint &cellPt,
                                const QVector<QPointF> &transformedPoints,
                                QPointF *srcPoint,
                                QPointF *dstPoint)
{
    QVector<PointExtension> extensionPoints;
    PointExtension ext;

    // left
    if ((ext.near = tryGetValidIndex(cellPt + QPoint(-1, 0))) >= 0 &&
        (ext.far = tryGetValidIndex(cellPt + QPoint(-2, 0))) >= 0) {

        extensionPoints << ext;
    }
    // top
    if ((ext.near = tryGetValidIndex(cellPt + QPoint(0, -1))) >= 0 &&
        (ext.far = tryGetValidIndex(cellPt + QPoint(0, -2))) >= 0) {

        extensionPoints << ext;
    }
    // right
    if ((ext.near = tryGetValidIndex(cellPt + QPoint(1, 0))) >= 0 &&
        (ext.far = tryGetValidIndex(cellPt + QPoint(2, 0))) >= 0) {

        extensionPoints << ext;
    }
    // bottom
    if ((ext.near = tryGetValidIndex(cellPt + QPoint(0, 1))) >= 0 &&
        (ext.far = tryGetValidIndex(cellPt + QPoint(0, 2))) >= 0) {

        extensionPoints << ext;
    }

    if (extensionPoints.isEmpty()) {
        // top-left
        if ((ext.near = tryGetValidIndex(cellPt + QPoint(-1, -1))) >= 0 &&
            (ext.far = tryGetValidIndex(cellPt + QPoint(-2, -2))) >= 0) {

            extensionPoints << ext;
        }
        // top-right
        if ((ext.near = tryGetValidIndex(cellPt + QPoint(1, -1))) >= 0 &&
            (ext.far = tryGetValidIndex(cellPt + QPoint(2, -2))) >= 0) {

            extensionPoints << ext;
        }
        // bottom-right
        if ((ext.near = tryGetValidIndex(cellPt + QPoint(1, 1))) >= 0 &&
            (ext.far = tryGetValidIndex(cellPt + QPoint(2, 2))) >= 0) {

            extensionPoints << ext;
        }
        // bottom-left
        if ((ext.near = tryGetValidIndex(cellPt + QPoint(-1, 1))) >= 0 &&
            (ext.far = tryGetValidIndex(cellPt + QPoint(-2, 2))) >= 0) {

            extensionPoints << ext;
        }
    }

    if (extensionPoints.isEmpty()) {
        return false;
    }

    int numResultPoints = 0;
    *srcPoint = allSrcPoints[pointToIndex(cellPt)];
    *dstPoint = QPointF();

    foreach (const PointExtension &ext, extensionPoints) {
        QPointF near = transformedPoints[ext.near];
        QPointF far = transformedPoints[ext.far];

        QPointF nearSrc = validPoints[ext.near];
        QPointF farSrc = validPoints[ext.far];

        QPointF base1 = nearSrc - farSrc;
        QPointF base2 = near - far;

        QPointF pt = near +
            KisAlgebra2D::transformAsBase(*srcPoint - nearSrc, base1, base2);

        *dstPoint += pt;
        numResultPoints++;
    }

    *dstPoint /= numResultPoints;

    return true;
}

inline QPoint KisCageTransformWorker::Private::
pointIndexToColRow(QPoint baseColRow, int index)
{
    static QVector<QPoint> pointOffsets;
    if (pointOffsets.isEmpty()) {
        pointOffsets << QPoint(0,0);
        pointOffsets << QPoint(1,0);
        pointOffsets << QPoint(1,1);
        pointOffsets << QPoint(0,1);
    }

    return baseColRow + pointOffsets[index];
}

template <class PolygonOp>
void KisCageTransformWorker::Private::
iterateThroughGrid(PolygonOp polygonOp,
                   const QVector<QPointF> &transformedPoints)
{
    QPolygonF cageDstPolygon(transfCage);
    QPolygonF cageSrcPolygon(origCage);

    QVector<int> polygonPoints(4);

    for (int row = 0; row < gridSize.height() - 1; row++) {
        for (int col = 0; col < gridSize.width() - 1; col++) {
            int numExistingPoints = 0;

            polygonPoints = calculateMappedIndexes(col, row, &numExistingPoints);

            if (numExistingPoints == 0) continue;

            if (numExistingPoints < 4) {

                QPolygonF srcPolygon;
                QPolygonF dstPolygon;

                for (int i = 0; i < 4; i++) {
                    const int index = polygonPoints[i];

                    if (index >= 0) {
                        srcPolygon << validPoints[index];
                        dstPolygon << transformedPoints[index];
                    } else {
                        QPoint cellPt = pointIndexToColRow(QPoint(col, row), i);
                        QPointF srcPoint;
                        QPointF dstPoint;
                        bool result =
                            getOrthogonalPointApproximation(cellPt,
                                                            transformedPoints,
                                                            &srcPoint,
                                                            &dstPoint);

                        if (!result) {
                            //qDebug() << "*NOT* found any valid point" << allSrcPoints[pointToIndex(cellPt)] << "->" << ppVar(pt);
                            break;
                        } else {
                            srcPolygon << srcPoint;
                            dstPolygon << dstPoint;
                        }
                    }
                }

                if (dstPolygon.size() == 4) {
                    QPolygonF srcClipPolygon(srcPolygon.intersected(cageSrcPolygon));

                    KisFourPointInterpolatorForward forwardTransform(srcPolygon, dstPolygon);
                    for (int i = 0; i < srcClipPolygon.size(); i++) {
                        const QPointF newPt = forwardTransform.map(srcClipPolygon[i]);
                        srcClipPolygon[i] = newPt;
                    }

                    polygonOp(srcPolygon, dstPolygon, srcClipPolygon);
                }
            } else {
                QPolygonF srcPolygon;
                QPolygonF dstPolygon;

                for (int i = 0; i < 4; i++) {
                    const int index = polygonPoints[i];
                    srcPolygon << validPoints[index];
                    dstPolygon << transformedPoints[index];
                }

                polygonOp(srcPolygon, dstPolygon);
            }
        }
    }
}

void KisCageTransformWorker::run()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->origCage.size() == m_d->transfCage.size());

    QVector<QPointF> transformedPoints = m_d->calculateTransformedPoints();

    KisPaintDeviceSP srcDev = new KisPaintDevice(*m_d->dev.data());

    {
        KisSelectionSP selection = new KisSelection();

        KisPainter painter(selection->pixelSelection());
        painter.setPaintColor(KoColor(Qt::black, selection->pixelSelection()->colorSpace()));
        painter.setAntiAliasPolygonFill(true);
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        painter.paintPolygon(m_d->origCage);

        m_d->dev->clearSelection(selection);
    }

    GridIterationTools::PaintDevicePolygonOp polygonOp(srcDev, m_d->dev);
    m_d->iterateThroughGrid(polygonOp, transformedPoints);
}

QImage KisCageTransformWorker::runOnQImage(QPointF *newOffset)
{
    KIS_ASSERT_RECOVER(m_d->origCage.size() == m_d->transfCage.size()) {
        return QImage();
    }

    KIS_ASSERT_RECOVER(!m_d->srcImage.isNull()) {
        return QImage();
    }

    KIS_ASSERT_RECOVER(m_d->srcImage.format() == QImage::Format_ARGB32) {
        return QImage();
    }

    QVector<QPointF> transformedPoints = m_d->calculateTransformedPoints();

    QRectF dstBounds;
    foreach (const QPointF &pt, transformedPoints) {
        kisAccumulateBounds(pt, &dstBounds);
    }

    const QRectF srcBounds(m_d->srcImageOffset, m_d->srcImage.size());
    dstBounds |= srcBounds;

    QPointF dstQImageOffset = dstBounds.topLeft();
    *newOffset = dstQImageOffset;

    QRect dstBoundsI = dstBounds.toAlignedRect();


    QImage dstImage(dstBoundsI.size(), m_d->srcImage.format());
    dstImage.fill(0);

    {
        // we shouldn't create too many painters
        QPainter gc(&dstImage);
        gc.drawImage(-dstQImageOffset + m_d->srcImageOffset, m_d->srcImage);
        gc.setBrush(Qt::black);
        gc.setPen(Qt::black);
        gc.setCompositionMode(QPainter::CompositionMode_Clear);
        gc.drawPolygon(QPolygonF(m_d->origCage).translated(-dstQImageOffset));
        gc.end();
    }

    GridIterationTools::QImagePolygonOp polygonOp(m_d->srcImage, dstImage, m_d->srcImageOffset, dstQImageOffset);
    m_d->iterateThroughGrid(polygonOp, transformedPoints);

    return dstImage;
}

