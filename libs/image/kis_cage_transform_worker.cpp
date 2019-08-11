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

#include <QPainter>

#include "KoColor.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "krita_utils.h"

#include <qnumeric.h>

struct Q_DECL_HIDDEN KisCageTransformWorker::Private
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

    bool isGridEmpty() const {
        return allSrcPoints.isEmpty();
    }


    QVector<QPointF> calculateTransformedPoints();

    inline QVector<int> calculateMappedIndexes(int col, int row,
                                               int *numExistingPoints);

    int tryGetValidIndex(const QPoint &cellPt);

    struct MapIndexesOp;
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

void KisCageTransformWorker::prepareTransform()
{
    if (m_d->origCage.size() < 3) return;

    const QPolygonF srcPolygon(m_d->origCage);

    QRect srcBounds = m_d->dev ? m_d->dev->region().boundingRect() :
        QRectF(m_d->srcImageOffset, m_d->srcImage.size()).toAlignedRect();
    srcBounds &= srcPolygon.boundingRect().toAlignedRect();

    // no need to process empty devices
    if (srcBounds.isEmpty()) return;
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

        if (qIsNaN(transformedPoints[i].x()) ||
            qIsNaN(transformedPoints[i].y())) {
            warnKrita << "WARNING: One grid point has been removed from consideration" << validPoints[i];
            transformedPoints[i] = validPoints[i];
        }

    }

    return transformedPoints;
}

inline QVector<int> KisCageTransformWorker::Private::
calculateMappedIndexes(int col, int row,
                       int *numExistingPoints)
{
    *numExistingPoints = 0;
    QVector<int> cellIndexes =
        GridIterationTools::calculateCellIndexes(col, row, gridSize);

    for (int i = 0; i < 4; i++) {
        cellIndexes[i] = allToValidPointsMap[cellIndexes[i]];
        *numExistingPoints += cellIndexes[i] >= 0;
    }

    return cellIndexes;
}



int KisCageTransformWorker::Private::
tryGetValidIndex(const QPoint &cellPt)
{
    int index = -1;
    if (cellPt.x() >= 0 &&
        cellPt.y() >= 0 &&
        cellPt.x() < gridSize.width() - 1 &&
        cellPt.y() < gridSize.height() - 1) {

        index = allToValidPointsMap[GridIterationTools::pointToIndex(cellPt, gridSize)];
    }

    return index;
}


struct KisCageTransformWorker::Private::MapIndexesOp {

    MapIndexesOp(KisCageTransformWorker::Private *d)
        : m_d(d),
          m_srcCagePolygon(QPolygonF(m_d->origCage))
    {
    }

    inline QVector<int> calculateMappedIndexes(int col, int row,
                                               int *numExistingPoints) const {

        return m_d->calculateMappedIndexes(col, row, numExistingPoints);
    }

    inline int tryGetValidIndex(const QPoint &cellPt) const {
        return m_d->tryGetValidIndex(cellPt);
    }

    inline QPointF getSrcPointForce(const QPoint &cellPt) const {
        return m_d->allSrcPoints[GridIterationTools::pointToIndex(cellPt, m_d->gridSize)];
    }

    inline const QPolygonF srcCropPolygon() const {
        return m_srcCagePolygon;
    }

    KisCageTransformWorker::Private *m_d;
    QPolygonF m_srcCagePolygon;
};

QRect KisCageTransformWorker::approxChangeRect(const QRect &rc)
{
    const qreal margin = 0.30;

    QVector<QPointF> cageSamplePoints;

    const int minStep = 3;
    const int maxSamples = 200;

    const int totalPixels = rc.width() * rc.height();
    const int realStep = qMax(minStep, totalPixels / maxSamples);
    const QPolygonF cagePolygon(m_d->origCage);

    for (int i = 0; i < totalPixels; i += realStep) {
        const int x = rc.x() + i % rc.width();
        const int y = rc.y() + i / rc.width();

        const QPointF pt(x, y);
        if (cagePolygon.containsPoint(pt, Qt::OddEvenFill)) {
            cageSamplePoints << pt;
        }
    }

    if (cageSamplePoints.isEmpty()) {
        return rc;
    }

    KisGreenCoordinatesMath cage;
    cage.precalculateGreenCoordinates(m_d->origCage, cageSamplePoints);
    cage.generateTransformedCageNormals(m_d->transfCage);

    const int numValidPoints = cageSamplePoints.size();
    QVector<QPointF> transformedPoints(numValidPoints);

    int failedPoints = 0;

    for (int i = 0; i < numValidPoints; i++) {
        transformedPoints[i] = cage.transformedPoint(i, m_d->transfCage);

        if (qIsNaN(transformedPoints[i].x()) ||
            qIsNaN(transformedPoints[i].y())) {

            transformedPoints[i] = cageSamplePoints[i];
            failedPoints++;
        }
    }

    QRect resultRect =
        KisAlgebra2D::approximateRectFromPoints(transformedPoints).toAlignedRect();

    return KisAlgebra2D::blowRect(resultRect | rc, margin);
}

QRect KisCageTransformWorker::approxNeedRect(const QRect &rc, const QRect &fullBounds)
{
    Q_UNUSED(rc);
    return fullBounds;
}

void KisCageTransformWorker::run()
{
    if (m_d->isGridEmpty()) return;

    KIS_ASSERT_RECOVER_RETURN(m_d->origCage.size() >= 3);
    KIS_ASSERT_RECOVER_RETURN(m_d->origCage.size() == m_d->transfCage.size());

    QVector<QPointF> transformedPoints = m_d->calculateTransformedPoints();

    KisPaintDeviceSP srcDev = new KisPaintDevice(*m_d->dev.data());
    KisPaintDeviceSP tempDevice = new KisPaintDevice(m_d->dev->colorSpace());

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

    GridIterationTools::PaintDevicePolygonOp polygonOp(srcDev, tempDevice);
    Private::MapIndexesOp indexesOp(m_d.data());
    GridIterationTools::iterateThroughGrid
        <GridIterationTools::IncompletePolygonPolicy>(polygonOp, indexesOp,
                                                      m_d->gridSize,
                                                      m_d->validPoints,
                                                      transformedPoints);

    QRect rect = tempDevice->extent();
    KisPainter gc(m_d->dev);
    gc.bitBlt(rect.topLeft(), tempDevice, rect);
}

QImage KisCageTransformWorker::runOnQImage(QPointF *newOffset)
{
    if (m_d->isGridEmpty()) return QImage();

    KIS_ASSERT_RECOVER(m_d->origCage.size() >= 3 &&
                       m_d->origCage.size() == m_d->transfCage.size()) {
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
    Q_FOREACH (const QPointF &pt, transformedPoints) {
        KisAlgebra2D::accumulateBounds(pt, &dstBounds);
    }

    const QRectF srcBounds(m_d->srcImageOffset, m_d->srcImage.size());
    dstBounds |= srcBounds;

    QPointF dstQImageOffset = dstBounds.topLeft();
    *newOffset = dstQImageOffset;

    QRect dstBoundsI = dstBounds.toAlignedRect();


    QImage dstImage(dstBoundsI.size(), m_d->srcImage.format());
    dstImage.fill(0);

    QImage tempImage(dstImage);

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

    GridIterationTools::QImagePolygonOp polygonOp(m_d->srcImage, tempImage, m_d->srcImageOffset, dstQImageOffset);
    Private::MapIndexesOp indexesOp(m_d.data());
    GridIterationTools::iterateThroughGrid
        <GridIterationTools::IncompletePolygonPolicy>(polygonOp, indexesOp,
                                                      m_d->gridSize,
                                                      m_d->validPoints,
                                                      transformedPoints);

    {
        QPainter gc(&dstImage);
        gc.drawImage(QPoint(), tempImage);
    }

    return dstImage;
}

