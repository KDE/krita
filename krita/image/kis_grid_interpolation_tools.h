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

#ifndef __KIS_GRID_INTERPOLATION_TOOLS_H
#define __KIS_GRID_INTERPOLATION_TOOLS_H

#include <limits>
#include <algorithm>

#include <QImage>

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
        KisSequentialIterator dstIt(m_dstDev, boundRect);
        KisRandomSubAccessorSP srcAcc = m_srcDev->createRandomSubAccessor();

        KisFourPointInterpolatorBackward interp(srcPolygon, dstPolygon);

        int y = boundRect.top();
        interp.setY(y);

        do {
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
                // transfomation we read data from "dstPoint"
                // (which is non-transformed) and write it into
                // "srcPoint" (which is transformed position)

                srcAcc->moveTo(dstPoint);
                srcAcc->sampledOldRawData(dstIt.rawData());
            }

        } while (dstIt.nextPixel());

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

                    srcPointI.rx() = qBound(m_dstImageRect.x(), srcPointI.x(), m_dstImageRect.right());
                    srcPointI.ry() = qBound(m_dstImageRect.y(), srcPointI.y(), m_dstImageRect.bottom());
                    dstPointI.rx() = qBound(m_srcImageRect.x(), dstPointI.x(), m_srcImageRect.right());
                    dstPointI.ry() = qBound(m_srcImageRect.y(), dstPointI.y(), m_srcImageRect.bottom());

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

}

#endif /* __KIS_GRID_INTERPOLATION_TOOLS_H */
