/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
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
#include "kis_painter.h"
#include "KisRegion.h"

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

        // we are erasing elements for not freeing the occupied
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

inline QPointF middlePoint(int x, int y) {
    return QPointF(x + 0.5, y + 0.5);
}

inline QPointF middlePoint(QPoint p) {
    return QPointF(p.x() + 0.5, p.y() + 0.5);
}

inline QPointF middlePoint(QPointF p) {
    return QPointF(p.x() + 0.5, p.y() + 0.5);
}

struct PaintDevicePolygonOp
{
    PaintDevicePolygonOp(KisPaintDeviceSP srcDev, KisPaintDeviceSP dstDev)
        : m_srcDev(srcDev), m_dstDev(dstDev) {}


    void fastCopyArea(QPolygonF areaToCopy) {
        QRect boundRect = areaToCopy.boundingRect().toAlignedRect();
        if (boundRect.isEmpty()) return;

        bool isItRect = KisAlgebra2D::isPolygonRect(areaToCopy, m_epsilon); // no need for lower tolerance
        if (isItRect) {
            fastCopyArea(boundRect);
            return;
        }

        KisSequentialIterator dstIt(m_dstDev, boundRect);
        KisSequentialIterator srcIt(m_srcDev, boundRect);

        // this can possibly be optimized with scanlining the polygon
        // (use intersectLineConvexPolygon to get a line at every height)
        // but it doesn't matter much because in the vast majority of cases
        // it should go straight to the rect area copying

        while (dstIt.nextPixel()  && srcIt.nextPixel()) {
            if (areaToCopy.containsPoint(middlePoint(dstIt.x(), dstIt.y()), Qt::OddEvenFill)) {
                memcpy(dstIt.rawData(), srcIt.oldRawData(), m_dstDev->pixelSize());
            }
        }
    }

    void fastCopyArea(QRect areaToCopy) {
        fastCopyArea(areaToCopy, m_canMergeRects);
    }

    void fastCopyArea(QRect areaToCopy, bool lazy) {
        if (lazy) {
            m_rectsToCopy.append(areaToCopy.adjusted(0, 0, -1, -1));
        } else {
            KisPainter::copyAreaOptimized(areaToCopy.topLeft(), m_srcDev, m_dstDev, areaToCopy);
        }
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        operator() (srcPolygon, dstPolygon, dstPolygon);
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon, const QPolygonF &clipDstPolygon) {
        QRect boundRect = clipDstPolygon.boundingRect().toAlignedRect();
        if (boundRect.isEmpty()) return;

        bool samePolygon = (m_dstDev->colorSpace() == m_srcDev->colorSpace()) && KisAlgebra2D::fuzzyPointCompare(srcPolygon, dstPolygon, m_epsilon);

        if (samePolygon) {
            // we can use clipDstPolygon here, because it will be smaller than dstPolygon and srcPolygon, because of how IncompletePolicy works
            // we could also calculate intersection here if we're worried whether that fact is always true
            fastCopyArea(clipDstPolygon);
            return;
        }


        KisSequentialIterator dstIt(m_dstDev, boundRect);
        KisRandomSubAccessorSP srcAcc = m_srcDev->createRandomSubAccessor();

        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        /**
         * We need to make sure that the destination polygon is not too small,
         * otherwise even small rounding will send the src-accessor into
         * infinity
         */
        if (interp.isValid(0.1)) {
            int y = boundRect.top();
            interp.setY(y);

            while (dstIt.nextPixel()) {
                int newY = dstIt.y();

                if (y != newY) {
                    y = newY;
                    interp.setY(y);
                }

                QPointF srcPoint(dstIt.x(), y);

                if (clipDstPolygon.containsPoint(middlePoint(srcPoint), Qt::OddEvenFill)) {

                    interp.setX(srcPoint.x());
                    QPointF dstPoint = interp.getValue();

                    // brain-blowing part:
                    //
                    // since the interpolator does the inverted
                    // transformation we read data from "dstPoint"
                    // (which is non-transformed) and write it into
                    // "srcPoint" (which is transformed position)

                    srcAcc->moveTo(dstPoint);
                    quint8* rawData = dstIt.rawData();
                    srcAcc->sampledOldRawData(rawData);
                }
            }

        } else {
            srcAcc->moveTo(interp.fallbackSourcePoint());

            while (dstIt.nextPixel()) {
                QPointF srcPoint(dstIt.x(), dstIt.y());

                if (clipDstPolygon.containsPoint(middlePoint(srcPoint), Qt::OddEvenFill)) {
                    srcAcc->sampledOldRawData(dstIt.rawData());
                }
            }
        }

    }

    void finalize() {

        QVector<QRect>::iterator end = KisRegion::mergeSparseRects(m_rectsToCopy.begin(), m_rectsToCopy.end());

        for (QVector<QRect>::iterator it = m_rectsToCopy.begin(); it < end; it++) {
            QRect areaToCopy = *it;
            fastCopyArea(areaToCopy.adjusted(0, 0, 1, 1), false);
        }
        m_rectsToCopy = QVector<QRect>();
    }

    inline void setCanMergeRects(bool newCanMergeRects) {
        m_canMergeRects = newCanMergeRects;
    }

    KisPaintDeviceSP m_srcDev;
    KisPaintDeviceSP m_dstDev;
    const qreal m_epsilon {0.1};

private:
    bool m_canMergeRects {true};
    QVector<QRect> m_rectsToCopy;

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

    void fastCopyArea(QPolygonF areaToCopy) {
        QRect boundRect = areaToCopy.boundingRect().toAlignedRect();

        if (boundRect.isEmpty()) return;

        bool isItRect = KisAlgebra2D::isPolygonRect(areaToCopy, m_epsilon); // no need for lower tolerance
        if (isItRect) {
            fastCopyArea(boundRect);
            return;
        }

        // this can possibly be optimized with scanlining the polygon
        // (use intersectLineConvexPolygon to get a line at every height)
        // but it doesn't matter much because in the vast majority of cases
        // it should go straight to the rect area copying

        for (int y = boundRect.top(); y <= boundRect.bottom(); y++) {
            for (int x = boundRect.left(); x <= boundRect.right(); x++) {
                QPointF dstPoint = QPointF(x, y);
                QPointF srcPoint = dstPoint;

                if (areaToCopy.containsPoint(middlePoint(srcPoint), Qt::OddEvenFill)) {

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

    }

    void fastCopyArea(QRect areaToCopy) {
        fastCopyArea(areaToCopy, m_canMergeRects);
    }

    void fastCopyArea(QRect areaToCopy, bool lazy) {
        if (lazy) {
            m_rectsToCopy.append(areaToCopy.adjusted(0, 0, -1, -1));
            return;
        }

        // only handling saved offsets
        QRect srcArea = areaToCopy.translated(-m_srcImageOffset.toPoint());
        QRect dstArea = areaToCopy.translated(-m_dstImageOffset.toPoint());

        srcArea = srcArea.intersected(m_srcImageRect);
        dstArea = dstArea.intersected(m_dstImageRect);

        // it might look pointless but it cuts off unneeded areas on both rects based on where they end up
        // since *I know* they are the same rectangle before translation
        // TODO: I'm pretty sure this logic is correct, but let's check it when I'm less sleepy
        QRect srcAreaUntranslated = srcArea.translated(m_srcImageOffset.toPoint());
        QRect dstAreaUntranslated = dstArea.translated(m_dstImageOffset.toPoint());

        QRect actualCopyArea = srcAreaUntranslated.intersected(dstAreaUntranslated);
        srcArea = actualCopyArea.translated(-m_srcImageOffset.toPoint());
        dstArea = actualCopyArea.translated(-m_dstImageOffset.toPoint());

        int bytesPerPixel = m_srcImage.sizeInBytes()/m_srcImage.height()/m_srcImage.width();

        int srcX = srcArea.left()*bytesPerPixel;
        int dstX = dstArea.left()*bytesPerPixel;

        for (int srcY = srcArea.top(); srcY <= srcArea.bottom(); ++srcY) {

            int dstY = dstArea.top() + srcY - srcArea.top();
            const uchar *srcLine = m_srcImage.constScanLine(srcY);
            uchar *dstLine = m_dstImage.scanLine(dstY);
            memcpy(dstLine + dstX, srcLine + srcX, srcArea.width()*bytesPerPixel);

        }
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        this->operator() (srcPolygon, dstPolygon, dstPolygon);
    }

    void operator() (const QPolygonF &srcPolygon, const QPolygonF &dstPolygon, const QPolygonF &clipDstPolygon) {
        QRect boundRect = clipDstPolygon.boundingRect().toAlignedRect();

        bool samePolygon = m_dstImage.format() == m_srcImage.format() && KisAlgebra2D::fuzzyPointCompare(srcPolygon, dstPolygon, m_epsilon);

        if (samePolygon) {
            // we can use clipDstPolygon here, because it will be smaller than dstPolygon and srcPolygon, because of how IncompletePolicy works
            // we could also calculate intersection here if we're worried whether that fact is always true
            fastCopyArea(clipDstPolygon);
            return;
        }

        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        for (int y = boundRect.top(); y <= boundRect.bottom(); y++) {
            interp.setY(y);
            for (int x = boundRect.left(); x <= boundRect.right(); x++) {

                QPointF srcPoint(x, y);
                if (clipDstPolygon.containsPoint(middlePoint(srcPoint), Qt::OddEvenFill)) {

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

    void finalize() {

        QVector<QRect>::iterator end = KisRegion::mergeSparseRects(m_rectsToCopy.begin(), m_rectsToCopy.end());

        for (QVector<QRect>::iterator it = m_rectsToCopy.begin(); it < end; it++) {
            QRect areaToCopy = *it;
            fastCopyArea(areaToCopy.adjusted(0, 0, 1, 1), false);
        }

        m_rectsToCopy = QVector<QRect>();
    }

    inline void setCanMergeRects(bool canMergeRects) {
        m_canMergeRects = canMergeRects;
    }


    const QImage &m_srcImage;
    QImage &m_dstImage;
    QPointF m_srcImageOffset;
    QPointF m_dstImageOffset;

    QRect m_srcImageRect;
    QRect m_dstImageRect;

    const qreal m_epsilon {0.1};

private:
    bool m_canMergeRects {true};
    QVector<QRect> m_rectsToCopy;
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

inline QRect calculateCorrectSubGrid(QRect originalBoundsForGrid, int pixelPrecision, QRectF currentBounds, QSize gridSize) {

    if (!QRectF(originalBoundsForGrid).intersects(currentBounds)) {
        return QRect();
    }

    QPoint offsetB = originalBoundsForGrid.topLeft();

    QPointF startPointB = currentBounds.topLeft() - offsetB;
    QPoint startPointG = QPoint(startPointB.x()/pixelPrecision, startPointB.y()/pixelPrecision);
    startPointG = QPoint(kisBoundFast(0, startPointG.x(), gridSize.width()), kisBoundFast(0, startPointG.y(), gridSize.height()));

    QPointF endPointB = currentBounds.bottomRight() + QPoint(1, 1) - offsetB; // *true* bottomRight
    QPoint endPointG = QPoint(std::ceil(endPointB.x()/pixelPrecision), std::ceil(endPointB.y()/pixelPrecision));
    QPoint endPointPotential = endPointG;

    QPoint trueEndPoint = QPoint(kisBoundFast(0, endPointPotential.x(), gridSize.width()), kisBoundFast(0, endPointPotential.y(), gridSize.height()));

    QPoint size = trueEndPoint - startPointG;

    return QRect(startPointG, QSize(size.x(), size.y()));
}

inline QList<QRectF> cutOutSubgridFromBounds(QRect subGrid, QRect srcBounds, const QSize &gridSize, const QVector<QPointF> &originalPoints) {
    if (subGrid.width() == 0 || subGrid.height() == 0) {
        return QList<QRectF> {srcBounds};
    }
    QPoint topLeft = subGrid.topLeft();
    QPoint bottomRight = subGrid.topLeft() + QPoint(subGrid.width() - 1, subGrid.height() - 1);

    int topLeftIndex = pointToIndex(topLeft, gridSize);
    int bottomRightIndex = pointToIndex(bottomRight, gridSize);

    topLeftIndex = qMax(0, qMin(topLeftIndex, originalPoints.length() - 1));
    bottomRightIndex = qMax(0, qMin(bottomRightIndex, originalPoints.length() - 1));

    QPointF topLeftReal = originalPoints[topLeftIndex];
    QPointF bottomRightReal = originalPoints[bottomRightIndex];
    QRectF cutOut = QRectF(topLeftReal, bottomRightReal);

    QList<QRectF> response;
    // *-----------*
    // |    top    |
    // |-----------|
    // | l |xxx| r |
    // | e |xxx| i |
    // | f |xxx| g |
    // | t |xxx| h |
    // |   |xxx| t |
    // |-----------|
    // |   bottom  |
    // *-----------*


    QRectF top = QRectF(srcBounds.topLeft(), QPointF(srcBounds.right() + 1, topLeftReal.y()));
    QRectF bottom = QRectF(QPointF(srcBounds.left(), bottomRightReal.y()), srcBounds.bottomRight() + QPointF(1, 1));
    QRectF left = QRectF(QPointF(srcBounds.left(), cutOut.top()), QPointF(cutOut.left(), cutOut.bottom()));
    QRectF right = QRectF(QPointF(cutOut.right(), cutOut.top()), QPointF(srcBounds.right() + 1, cutOut.bottom()));
    QList<QRectF> rects = {top, left, right, bottom};
    for (int i = 0; i < rects.length(); i++) {
        if (!rects[i].isEmpty()) {
            response << rects[i];
        }
    }
    return response;

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

template <class IndexesOp>
bool canProcessRectsInRandomOrder(IndexesOp &indexesOp, const QVector<QPointF> &transformedPoints, QSize grid) {
    return canProcessRectsInRandomOrder(indexesOp, transformedPoints, QRect(QPoint(0, 0), grid));
}

template <class IndexesOp>
bool canProcessRectsInRandomOrder(IndexesOp &indexesOp, const QVector<QPointF> &transformedPoints, QRect subgrid) {
    QVector<int> polygonPoints(4);
    QPoint startPoint = subgrid.topLeft();
    QPoint endPoint = subgrid.bottomRight();


    int polygonsChecked = 0;

    for (int row = startPoint.y(); row < endPoint.y(); row++) {
        for (int col = startPoint.x(); col < endPoint.x(); col++) {
            int numExistingPoints = 0;

            polygonPoints = indexesOp.calculateMappedIndexes(col, row, &numExistingPoints);

            QPolygonF dstPolygon;

            for (int i = 0; i < polygonPoints.count(); i++) {
                const int index = polygonPoints[i];
                dstPolygon << transformedPoints[index];
            }


            adjustAlignedPolygon(dstPolygon);


            if (!KisAlgebra2D::isPolygonTrulyConvex(dstPolygon)) {
                return false;
            }

        }
    }
    return true;
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
    iterateThroughGrid<IncompletePolygonPolicy, PolygonOp, IndexesOp>(polygonOp, indexesOp, gridSize, originalPoints, transformedPoints, QRect(QPoint(0, 0), gridSize));
}

template <template <class PolygonOp, class IndexesOp> class IncompletePolygonPolicy,
          class PolygonOp,
          class IndexesOp>
void iterateThroughGrid(PolygonOp &polygonOp,
                        IndexesOp &indexesOp,
                        const QSize &gridSize,
                        const QVector<QPointF> &originalPoints,
                        const QVector<QPointF> &transformedPoints,
                        const QRect subGrid)
{
    QVector<int> polygonPoints(4);
    QPoint startPoint = subGrid.topLeft();
    QPoint endPoint = subGrid.bottomRight(); // it's weird but bottomRight on QRect point does give us one unit of margin on both x and y
    // when start is on (0, 0), and size is (500, 500), bottomRight is on (499, 499)
    // but remember that it also only needs a top left corner of the polygon

    KIS_SAFE_ASSERT_RECOVER(startPoint.x() >= 0 && startPoint.y() >= 0 && endPoint.x() <= gridSize.width() - 1 && endPoint.y() <= gridSize.height() - 1) {
        startPoint = QPoint(qMax(startPoint.x(), 0), qMax(startPoint.y(), 0));
        endPoint = QPoint(qMin(endPoint.x(), gridSize.width() - 1), qMin(startPoint.y(), gridSize.height() - 1));
    }

    for (int row = startPoint.y(); row < endPoint.y(); row++) {
        for (int col = startPoint.x(); col < endPoint.x(); col++) {
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

    polygonOp.finalize();
}

}

#endif /* __KIS_GRID_INTERPOLATION_TOOLS_H */
