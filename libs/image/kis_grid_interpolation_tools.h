/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GRID_INTERPOLATION_TOOLS_H
#define __KIS_GRID_INTERPOLATION_TOOLS_H

#include <limits>
#include <algorithm>

#include <QImage>

#include "kis_algebra_2d.h"
#include "kis_four_point_interpolator_forward.h"
#include "kis_four_point_interpolator_backward.h"
#include "kis_iterator_ng.h"
#include "kis_random_sub_accessor.h"

//#define DEBUG_PAINTING_POLYGONS

#ifdef DEBUG_PAINTING_POLYGONS
#include <QPainter>
#endif /* DEBUG_PAINTING_POLYGONS */

namespace GridIterationTools {

inline int calcGridDimension(int start, int end, const int pixelPrecision)
{
    const int alignmentMask = ~(pixelPrecision - 1);

    int alignedStart = (start + pixelPrecision - 1) & alignmentMask;
    int alignedEnd = end & alignmentMask;

    int size = 0;

    if (alignedEnd > alignedStart) {
        size = (alignedEnd - alignedStart) / pixelPrecision + 1;
        size += alignedStart != start;
        size += alignedEnd != end;
    } else {
        size = 2 + (end - start >= pixelPrecision);
    }

    return size;
}

inline QSize calcGridSize(const QRect &srcBounds, const int pixelPrecision) {
    return QSize(calcGridDimension(srcBounds.x(), srcBounds.right(), pixelPrecision),
                 calcGridDimension(srcBounds.y(), srcBounds.bottom(), pixelPrecision));
}

template <class ProcessPolygon, class ForwardTransform>
struct CellOp
{
    CellOp(ProcessPolygon &_polygonOp, ForwardTransform &_transformOp)
        : polygonOp(_polygonOp),
          transformOp(_transformOp)
    {
    }

    inline void processPoint(int col, int row,
                             int prevCol, int prevRow,
                             int colIndex, int rowIndex) {

        QPointF dstPosF = transformOp(QPointF(col, row));
        currLinePoints << dstPosF;

        if (rowIndex >= 1 && colIndex >= 1) {
            QPolygonF srcPolygon;

            srcPolygon << QPointF(prevCol, prevRow);
            srcPolygon << QPointF(col, prevRow);
            srcPolygon << QPointF(col, row);
            srcPolygon << QPointF(prevCol, row);

            QPolygonF dstPolygon;

            dstPolygon << prevLinePoints.at(colIndex - 1);
            dstPolygon << prevLinePoints.at(colIndex);
            dstPolygon << currLinePoints.at(colIndex);
            dstPolygon << currLinePoints.at(colIndex - 1);

            polygonOp(srcPolygon, dstPolygon);
        }

    }

    inline void nextLine() {
        std::swap(prevLinePoints, currLinePoints);

        // we are erasing elements for not free'ing the occupied
        // memory, which is more efficient since we are going to fill
        // the vector again
        currLinePoints.erase(currLinePoints.begin(), currLinePoints.end());
    }

    QVector<QPointF> prevLinePoints;
    QVector<QPointF> currLinePoints;
    ProcessPolygon &polygonOp;
    ForwardTransform &transformOp;
};

template <class ProcessCell>
void processGrid(ProcessCell &cellOp,
                 const QRect &srcBounds,
                 const int pixelPrecision)
{
    if (srcBounds.isEmpty()) return;

    const int alignmentMask = ~(pixelPrecision - 1);

    int prevRow = std::numeric_limits<int>::max();
    int prevCol = std::numeric_limits<int>::max();

    int rowIndex = 0;
    int colIndex = 0;

    for (int row = srcBounds.top(); row <= srcBounds.bottom();) {
        for (int col = srcBounds.left(); col <= srcBounds.right();) {

            cellOp.processPoint(col, row,
                                prevCol, prevRow,
                                colIndex, rowIndex);

            prevCol = col;
            col += pixelPrecision;
            colIndex++;

            if (col > srcBounds.right() &&
                col <= srcBounds.right() + pixelPrecision - 1) {

                col = srcBounds.right();
            } else {
                col &= alignmentMask;
            }
        }

        cellOp.nextLine();
        colIndex = 0;

        prevRow = row;
        row += pixelPrecision;
        rowIndex++;

        if (row > srcBounds.bottom() &&
            row <= srcBounds.bottom() + pixelPrecision - 1) {

            row = srcBounds.bottom();
        } else {
            row &= alignmentMask;
        }
    }
}

template <class ProcessPolygon, class ForwardTransform>
void processGrid(ProcessPolygon &polygonOp, ForwardTransform &transformOp,
                 const QRect &srcBounds, const int pixelPrecision)
{
    CellOp<ProcessPolygon, ForwardTransform> cellOp(polygonOp, transformOp);
    processGrid(cellOp, srcBounds, pixelPrecision);
}

struct PaintDevicePolygonOp
{
    PaintDevicePolygonOp(KisPaintDeviceSP srcDev, KisPaintDeviceSP dstDev)
        : m_srcDev(srcDev), m_dstDev(dstDev) {}

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        this->operator() (srcPolygon, dstPolygon, dstPolygon);
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon, const QPolygonF &clipDstPolygon) {
        QRect boundRect = clipDstPolygon.boundingRect().toAlignedRect();
        if (boundRect.isEmpty()) return;

        KisSequentialIterator dstIt(m_dstDev, boundRect);
        KisRandomSubAccessorSP srcAcc = m_srcDev->createRandomSubAccessor();

        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        int y = boundRect.top();
        interp.setY(y);

        while (dstIt.nextPixel()) {
            int newY = dstIt.y();

            if (y != newY) {
                y = newY;
                interp.setY(y);
            }

            QPointF srcPoint(dstIt.x(), y);

            if (clipDstPolygon.containsPoint(srcPoint, Qt::OddEvenFill)) {

                interp.setX(srcPoint.x());
                QPointF dstPoint = interp.getValue();

                // brain-blowing part:
                //
                // since the interpolator does the inverted
                // transformation we read data from "dstPoint"
                // (which is non-transformed) and write it into
                // "srcPoint" (which is transformed position)

                srcAcc->moveTo(dstPoint);
                srcAcc->sampledOldRawData(dstIt.rawData());
            }

        }

    }

    KisPaintDeviceSP m_srcDev;
    KisPaintDeviceSP m_dstDev;
};

struct QImagePolygonOp
{
    QImagePolygonOp(const QImage &srcImage, QImage &dstImage,
                    const QPointF &srcImageOffset,
                    const QPointF &dstImageOffset)
        : m_srcImage(srcImage), m_dstImage(dstImage),
          m_srcImageOffset(srcImageOffset),
          m_dstImageOffset(dstImageOffset),
          m_srcImageRect(m_srcImage.rect()),
          m_dstImageRect(m_dstImage.rect())
    {
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        this->operator() (srcPolygon, dstPolygon, dstPolygon);
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon, const QPolygonF &clipDstPolygon) {
        QRect boundRect = clipDstPolygon.boundingRect().toAlignedRect();
        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        for (int y = boundRect.top(); y <= boundRect.bottom(); y++) {
            interp.setY(y);
            for (int x = boundRect.left(); x <= boundRect.right(); x++) {

                QPointF srcPoint(x, y);
                if (clipDstPolygon.containsPoint(srcPoint, Qt::OddEvenFill)) {

                    interp.setX(srcPoint.x());
                    QPointF dstPoint = interp.getValue();

                    // about srcPoint/dstPoint hell please see a
                    // comment in PaintDevicePolygonOp::operator() ()

                    srcPoint -= m_dstImageOffset;
                    dstPoint -= m_srcImageOffset;

                    QPoint srcPointI = srcPoint.toPoint();
                    QPoint dstPointI = dstPoint.toPoint();

                    if (!m_dstImageRect.contains(srcPointI)) continue;
                    if (!m_srcImageRect.contains(dstPointI)) continue;

                    m_dstImage.setPixel(srcPointI, m_srcImage.pixel(dstPointI));
                }
            }
        }

#ifdef DEBUG_PAINTING_POLYGONS
        QPainter gc(&m_dstImage);
        gc.setPen(Qt::red);
        gc.setOpacity(0.5);

        gc.setBrush(Qt::green);
        gc.drawPolygon(clipDstPolygon.translated(-m_dstImageOffset));

        gc.setBrush(Qt::blue);
        //gc.drawPolygon(dstPolygon.translated(-m_dstImageOffset));

#endif /* DEBUG_PAINTING_POLYGONS */

    }

    const QImage &m_srcImage;
    QImage &m_dstImage;
    QPointF m_srcImageOffset;
    QPointF m_dstImageOffset;

    QRect m_srcImageRect;
    QRect m_dstImageRect;
};

/*************************************************************/
/*      Iteration through precalculated grid                 */
/*************************************************************/

/**
 *    A-----B         The polygons will be in the following order:
 *    |     |
 *    |     |         polygon << A << B << D << C;
 *    C-----D
 */
inline QVector<int> calculateCellIndexes(int col, int row, const QSize &gridSize)
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

inline int pointToIndex(const QPoint &cellPt, const QSize &gridSize)
{
    return cellPt.x() +
        cellPt.y() * gridSize.width();
}

namespace Private {
    inline QPoint pointPolygonIndexToColRow(QPoint baseColRow, int index)
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

    struct PointExtension {
        int near;
        int far;
    };
}

template <class IndexesOp>
bool getOrthogonalPointApproximation(const QPoint &cellPt,
                                const QVector<QPointF> &originalPoints,
                                const QVector<QPointF> &transformedPoints,
                                IndexesOp indexesOp,
                                QPointF *srcPoint,
                                QPointF *dstPoint)
{
    QVector<Private::PointExtension> extensionPoints;
    Private::PointExtension ext;

    // left
    if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(-1, 0))) >= 0 &&
        (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(-2, 0))) >= 0) {

        extensionPoints << ext;
    }
    // top
    if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(0, -1))) >= 0 &&
        (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(0, -2))) >= 0) {

        extensionPoints << ext;
    }
    // right
    if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(1, 0))) >= 0 &&
        (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(2, 0))) >= 0) {

        extensionPoints << ext;
    }
    // bottom
    if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(0, 1))) >= 0 &&
        (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(0, 2))) >= 0) {

        extensionPoints << ext;
    }

    if (extensionPoints.isEmpty()) {
        // top-left
        if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(-1, -1))) >= 0 &&
            (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(-2, -2))) >= 0) {

            extensionPoints << ext;
        }
        // top-right
        if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(1, -1))) >= 0 &&
            (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(2, -2))) >= 0) {

            extensionPoints << ext;
        }
        // bottom-right
        if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(1, 1))) >= 0 &&
            (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(2, 2))) >= 0) {

            extensionPoints << ext;
        }
        // bottom-left
        if ((ext.near = indexesOp.tryGetValidIndex(cellPt + QPoint(-1, 1))) >= 0 &&
            (ext.far = indexesOp.tryGetValidIndex(cellPt + QPoint(-2, 2))) >= 0) {

            extensionPoints << ext;
        }
    }

    if (extensionPoints.isEmpty()) {
        return false;
    }

    int numResultPoints = 0;
    *srcPoint = indexesOp.getSrcPointForce(cellPt);
    *dstPoint = QPointF();

    Q_FOREACH (const Private::PointExtension &ext, extensionPoints) {
        QPointF near = transformedPoints[ext.near];
        QPointF far = transformedPoints[ext.far];

        QPointF nearSrc = originalPoints[ext.near];
        QPointF farSrc = originalPoints[ext.far];

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

template <class PolygonOp, class IndexesOp>
struct IncompletePolygonPolicy {

    static inline bool tryProcessPolygon(int col, int row,
                                         int numExistingPoints,
                                         PolygonOp &polygonOp,
                                         IndexesOp &indexesOp,
                                         const QVector<int> &polygonPoints,
                                         const QVector<QPointF> &originalPoints,
                                         const QVector<QPointF> &transformedPoints)
    {
        if (numExistingPoints >= 4) return false;
        if (numExistingPoints == 0) return true;

        QPolygonF srcPolygon;
        QPolygonF dstPolygon;

        for (int i = 0; i < 4; i++) {
            const int index = polygonPoints[i];

            if (index >= 0) {
                srcPolygon << originalPoints[index];
                dstPolygon << transformedPoints[index];
            } else {
                QPoint cellPt = Private::pointPolygonIndexToColRow(QPoint(col, row), i);
                QPointF srcPoint;
                QPointF dstPoint;
                bool result =
                    getOrthogonalPointApproximation(cellPt,
                                                    originalPoints,
                                                    transformedPoints,
                                                    indexesOp,
                                                    &srcPoint,
                                                    &dstPoint);

                if (!result) {
                    //dbgKrita << "*NOT* found any valid point" << allSrcPoints[pointToIndex(cellPt)] << "->" << ppVar(pt);
                    break;
                } else {
                    srcPolygon << srcPoint;
                    dstPolygon << dstPoint;
                }
            }
        }

        if (dstPolygon.size() == 4) {
            QPolygonF srcClipPolygon(srcPolygon.intersected(indexesOp.srcCropPolygon()));

            KisFourPointInterpolatorForward forwardTransform(srcPolygon, dstPolygon);
            for (int i = 0; i < srcClipPolygon.size(); i++) {
                const QPointF newPt = forwardTransform.map(srcClipPolygon[i]);
                srcClipPolygon[i] = newPt;
            }

            polygonOp(srcPolygon, dstPolygon, srcClipPolygon);
        }

        return true;
    }
};

template <class PolygonOp, class IndexesOp>
struct AlwaysCompletePolygonPolicy {

    static inline bool tryProcessPolygon(int col, int row,
                                         int numExistingPoints,
                                         PolygonOp &polygonOp,
                                         IndexesOp &indexesOp,
                                         const QVector<int> &polygonPoints,
                                         const QVector<QPointF> &originalPoints,
                                         const QVector<QPointF> &transformedPoints)
    {
        Q_UNUSED(col);
        Q_UNUSED(row);
        Q_UNUSED(polygonOp);
        Q_UNUSED(indexesOp);
        Q_UNUSED(polygonPoints);
        Q_UNUSED(originalPoints);
        Q_UNUSED(transformedPoints);

        KIS_ASSERT_RECOVER_NOOP(numExistingPoints == 4);
        return false;
    }
};

struct RegularGridIndexesOp {

    RegularGridIndexesOp(const QSize &gridSize)
        : m_gridSize(gridSize)
    {
    }

    inline QVector<int> calculateMappedIndexes(int col, int row,
                                               int *numExistingPoints) const {

        *numExistingPoints = 4;
        QVector<int> cellIndexes =
            GridIterationTools::calculateCellIndexes(col, row, m_gridSize);

        return cellIndexes;
    }

    inline int tryGetValidIndex(const QPoint &cellPt) const {
        Q_UNUSED(cellPt);

        KIS_ASSERT_RECOVER_NOOP(0 && "Not applicable");
        return -1;
    }

    inline QPointF getSrcPointForce(const QPoint &cellPt) const {
        Q_UNUSED(cellPt);

        KIS_ASSERT_RECOVER_NOOP(0 && "Not applicable");
        return QPointF();
    }

    inline const QPolygonF srcCropPolygon() const {
        KIS_ASSERT_RECOVER_NOOP(0 && "Not applicable");
        return QPolygonF();
    }

    QSize m_gridSize;
};

/**
 * There is a weird problem in fetching correct bounds of the polygon.
 * If the rightmost (bottommost) point of the polygon is integral, then
 * QRectF() will end exactly on it, but when converting into QRect the last
 * point will not be taken into account. It happens due to the difference
 * between center-point/topleft-point point representation. In many cases
 * the latter is expected, but we don't work with it in Qt/Krita.
 */
inline void adjustAlignedPolygon(QPolygonF &polygon)
{
    static const qreal eps = 1e-5;
    static const  QPointF p1(eps, 0.0);
    static const  QPointF p2(eps, eps);
    static const  QPointF p3(0.0, eps);

    polygon[1] += p1;
    polygon[2] += p2;
    polygon[3] += p3;
}

template <template <class PolygonOp, class IndexesOp> class IncompletePolygonPolicy,
          class PolygonOp,
          class IndexesOp>
void iterateThroughGrid(PolygonOp &polygonOp,
                        IndexesOp &indexesOp,
                        const QSize &gridSize,
                        const QVector<QPointF> &originalPoints,
                        const QVector<QPointF> &transformedPoints)
{
    QVector<int> polygonPoints(4);

    for (int row = 0; row < gridSize.height() - 1; row++) {
        for (int col = 0; col < gridSize.width() - 1; col++) {
            int numExistingPoints = 0;

            polygonPoints = indexesOp.calculateMappedIndexes(col, row, &numExistingPoints);

            if (!IncompletePolygonPolicy<PolygonOp, IndexesOp>::
                 tryProcessPolygon(col, row,
                                   numExistingPoints,
                                   polygonOp,
                                   indexesOp,
                                   polygonPoints,
                                   originalPoints,
                                   transformedPoints)) {

                QPolygonF srcPolygon;
                QPolygonF dstPolygon;

                for (int i = 0; i < 4; i++) {
                    const int index = polygonPoints[i];
                    srcPolygon << originalPoints[index];
                    dstPolygon << transformedPoints[index];
                }

                adjustAlignedPolygon(srcPolygon);
                adjustAlignedPolygon(dstPolygon);

                polygonOp(srcPolygon, dstPolygon);
            }
        }
    }
}

}

#endif /* __KIS_GRID_INTERPOLATION_TOOLS_H */
