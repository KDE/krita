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

#include "KoColor.h"
#include "kis_selection.h"
#include "kis_painter.h"


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
    QVector<QPointF> origCage;
    QVector<QPointF> transfCage;
    KoUpdater *progress;
    int pixelPrecision;

    QVector<int> allToValidPointsMap;
    QVector<QPointF> validPoints;

    KisGreenCoordinatesMath cage;

    QSize gridSize;
};

KisCageTransformWorker::KisCageTransformWorker(KisPaintDeviceSP dev,
                                               const QVector<QPointF> &origCage,
                                               KoUpdater *progress,
                                               int pixelPrecision)
    : m_d(new Private(dev, origCage, progress, pixelPrecision))
{
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
            m_numValidPoints++;
        } else {
            m_points << invalidPoint;
        }
    }

    inline void nextLine() {
    }

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
    const QRect srcBounds = m_d->dev->region().boundingRect() &
        srcPolygon.boundingRect().toAlignedRect();

    m_d->gridSize =
        GridIterationTools::calcGridSize(srcBounds, m_d->pixelPrecision);

    PointsFetcherOp pointsOp(srcPolygon);
    GridIterationTools::processGrid(pointsOp, srcBounds, m_d->pixelPrecision);

    const int numPoints = pointsOp.m_points.size();

    KIS_ASSERT_RECOVER_RETURN(numPoints == m_d->gridSize.width() * m_d->gridSize.height());

    m_d->allToValidPointsMap.resize(pointsOp.m_points.size());
    m_d->validPoints.resize(pointsOp.m_numValidPoints);

    {
        int validIdx = 0;
        for (int i = 0; i < numPoints; i++) {
            const QPointF &pt = pointsOp.m_points[i];

            if (pointsOp.isPointValid(pt)) {
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

void KisCageTransformWorker::run()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->origCage.size() == m_d->transfCage.size());

    m_d->cage.generateTransformedCageNormals(m_d->transfCage);

    const int numValidPoints = m_d->validPoints.size();
    QVector<QPointF> transformedPoints(numValidPoints);

    for (int i = 0; i < numValidPoints; i++) {
        transformedPoints[i] = m_d->cage.transformedPoint(i, m_d->transfCage);
    }

    // here the cage is not needed anymore

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

    QVector<int> polygonPoints(4);

    for (int row = 0; row < m_d->gridSize.height() - 1; row++) {
        for (int col = 0; col < m_d->gridSize.width() - 1; col++) {
            polygonPoints[0] = m_d->allToValidPointsMap[col + row * m_d->gridSize.width()];
            if (polygonPoints[0] < 0) continue;
            polygonPoints[1] = m_d->allToValidPointsMap[col + 1 + row * m_d->gridSize.width()];
            if (polygonPoints[1] < 0) continue;
            polygonPoints[2] = m_d->allToValidPointsMap[col + 1 + (row + 1) * m_d->gridSize.width()];
            if (polygonPoints[2] < 0) continue;
            polygonPoints[3] = m_d->allToValidPointsMap[col + (row + 1) * m_d->gridSize.width()];
            if (polygonPoints[3] < 0) continue;

            QPolygonF srcPolygon;
            QPolygonF dstPolygon;

            for (int i = 0; i < 4; i++) {
                const int index = polygonPoints[i];
                srcPolygon << m_d->validPoints[index];
                dstPolygon << transformedPoints[index];
            }

            //qDebug() << ppVar(col) << ppVar(row);
            //qDebug() << ppVar(srcPolygon);
            //qDebug() << ppVar(dstPolygon);

            polygonOp(srcPolygon, dstPolygon);
        }
    }
}
