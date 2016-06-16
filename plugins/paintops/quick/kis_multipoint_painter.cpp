/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_multipoint_painter.h"

#include <limits>

#include <QtCore/qmath.h>

#include <QRegion>

#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_debug.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_random_accessor_ng.h"


void KisMultipointPainter::PointArea::initDxDy(const QPointF &center, float rx, float ry)
{
    const int width = xEnd - xStart + 1;
    const int height = yEnd - yStart + 1;

    dxValues.resize(width);
    dyValues.resize(height);

    const int cx = center.x();
    const int cy = center.y();

    const float rx2inv = baseUnitValue / pow2(rx);
    const float ry2inv = baseUnitValue / pow2(ry);

    for (int i = 0; i < width; i++) {
        dxValues[i] = rx2inv * pow2(cx - i - xStart)/* + (1.0 - baseUnitValue)*/;
    }

    for (int i = 0; i < height; i++) {
        dyValues[i] = ry2inv * pow2(cy - i - yStart);
    }

    //baseUnitValue = 1.0;
}

KisMultipointPainter::PointRow
KisMultipointPainter::PointArea::fillRow(int i) const
{
    Q_ASSERT(i >= yStart && i <= yEnd);

    PointRow row;

    row.start = xStart;
    row.end = xEnd;

    row.dxValues = &dxValues;
    row.dyValue = dyValues[i - yStart];

    row.baseUnitValue = baseUnitValue;

    return row;
}

struct KisMultipointPainter::Private
{
    QVector<Point> points;

    QRect boundingRect;
    QRegion dirtyRegion;

    QVector<PointArea> convertedPoints;

    QVector<MergedArea> mergedAreas;

    KisRandomAccessorSP dstIt;

    void clear() {
        points.clear();
        boundingRect = QRect();
        dirtyRegion = QRegion();
        convertedPoints.clear();
        mergedAreas.clear();
    }
};


KisMultipointPainter::KisMultipointPainter()
    : m_d(new Private)
{
}

KisMultipointPainter::~KisMultipointPainter()
{
}

inline void writeIfMin(int x, int *min)
{
    if (x < *min) {
        *min = x;
    }
}

inline void writeIfMax(int x, int *max)
{
    if (x > *max) {
        *max = x;
    }
}

void KisMultipointPainter::setPoints(const QVector<Point> &points)
{
    m_d->clear();

    m_d->points = points;

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();

    foreach (const Point &pt, points) {
        const int x0 = qFloor(pt.pos.x() - pt.rx);
        const int x1 = qCeil(pt.pos.x() + pt.rx);
        const int y0 = qFloor(pt.pos.y() - pt.ry);
        const int y1 = qCeil(pt.pos.y() + pt.ry);

        writeIfMin(x0, &minX);
        writeIfMax(x1, &maxX);

        writeIfMin(y0, &minY);
        writeIfMax(y1, &maxY);


        //m_d->dirtyRegion += QRect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);

        PointArea area;

        area.xStart = x0;
        area.xEnd = x1;
        area.yStart = y0;
        area.yEnd = y1;
        area.baseUnitValue = pt.opacity;

        area.initDxDy(pt.pos, pt.rx, pt.ry);

        m_d->convertedPoints << area;
    }

    m_d->boundingRect = QRect(minX, minY,
                              maxX - minX + 1,
                              maxY - minY + 1);

}

QRect KisMultipointPainter::boundingRect() const
{
    return m_d->boundingRect;
}

KisMultipointPainter::CompositeRow
KisMultipointPainter::getCompositeRow(int y)
{
    CompositeRow row;

    row.start = std::numeric_limits<int>::max();
    row.end = std::numeric_limits<int>::min();

    foreach (const PointArea &pt, m_d->convertedPoints) {
        if (pt.yStart <= y && y <= pt.yEnd) {
            writeIfMin(pt.xStart, &row.start);
            writeIfMax(pt.xEnd, &row.end);

            row.rows.append(pt.fillRow(y));
        }
    }

    return row;
}

#if 0
void KisMultipointPainter::calcMergedAreas()
{
    QVector<const PointArea*> pointsCopy;
    foreach(const PointArea &pt, m_d->convertedPoints) {
        pointsCopy.append(&pt);
    }

    MergedArea *currentArea = 0;

    const int y0 = m_d->boundingRect.top();
    const int y1 = m_d->boundingRect.bottom();

    QVector<const PointArea*> lastActivePoints;

    for (int y = y0; y <= y1; y++) {

        QVector<const PointArea*> activePoints;

        QVector<const PointArea*>::iterator it = pointsCopy.begin();

        while (it != pointsCopy.end()) {
            const PointArea *pt = *it;

            if (pt->yEnd < y) {
                it = pointsCopy.erase(it);
                continue;
            }

            if (pt->yStart <= y) {
                activePoints.append(pt);
            }

            ++it;
        }

        if (activePoints != lastActivePoints) {
            QVector<const PointArea*>::const_iterator it;

            int minX = std::numeric_limits<int>::max();
            int maxX = std::numeric_limits<int>::min();

            it = activePoints.constBegin();
            for (; it != activePoints.constEnd(); ++it) {
                const PointArea *pt = *it;

                writeIfMin(pt->xStart, &minX);
                writeIfMax(pt->xEnd, &maxX);
            }

            m_d->mergedAreas << MergedArea();
            currentArea = &m_d->mergedAreas.last();

            currentArea->xStart = minX;
            currentArea->xEnd = maxX;
            currentArea->yStart = y;
            currentArea->yEnd = y;

            currentArea->activePoints = activePoints;

            const int width = maxX - minX + 1;

            //currentArea->baseUnitValues.resize(width);
            currentArea->dxValues.resize(width);

            for (int j = minX; j <= maxX; j++) {
                it = activePoints.constBegin();
                for (; it != activePoints.constEnd(); ++it) {
                    const PointArea *pt = *it;

                    if (j < pt->xStart || pt->xEnd < j) continue;

                    //currentArea->baseUnitValues[j - minX] += pt->baseUnitValue;
                    currentArea->dxValues[j - minX] += pt->dxValues[j - pt->xStart];
                }
            }

            currentArea->dyValues.append(0.0);

            it = activePoints.constBegin();
            for (; it != activePoints.constEnd(); ++it) {
                const PointArea *pt = *it;

                currentArea->dyValues.last() += pt->dyValues[y - pt->yStart];
            }

            lastActivePoints = activePoints;

        } else if (currentArea) {
            currentArea->yEnd = y;

            currentArea->dyValues.append(0.0);

            QVector<const PointArea*>::const_iterator it;
            it = lastActivePoints.constBegin();
            for (; it != lastActivePoints.constEnd(); ++it) {
                const PointArea *pt = *it;

                currentArea->dyValues.last() += pt->dyValues[y - pt->yStart];
            }
        }
    }
}
#endif

typedef KisMultipointPainter::PointArea PointArea;
typedef KisMultipointPainter::MergedArea MergedArea;

struct HorizontalWrapper
{
    const int& mainStart(const PointArea *pt) { return pt->xStart; }
    const int& mainEnd(const PointArea *pt) { return pt->xEnd; }
    const int& otherStart(const PointArea *pt) { return pt->yStart; }
    const int& otherEnd(const PointArea *pt) { return pt->yEnd; }

    int& mainStart(MergedArea *pt) { return pt->xStart; }
    int& mainEnd(MergedArea *pt) { return pt->xEnd; }
    int& otherStart(MergedArea *pt) { return pt->yStart; }
    int& otherEnd(MergedArea *pt) { return pt->yEnd; }
};

struct VerticalWrapper
{
    const int& mainStart(const PointArea *pt) { return pt->yStart; }
    const int& mainEnd(const PointArea *pt) { return pt->yEnd; }
    const int& otherStart(const PointArea *pt) { return pt->xStart; }
    const int& otherEnd(const PointArea *pt) { return pt->xEnd; }

    int& mainStart(MergedArea *pt) { return pt->yStart; }
    int& mainEnd(MergedArea *pt) { return pt->yEnd; }
    int& otherStart(MergedArea *pt) { return pt->xStart; }
    int& otherEnd(MergedArea *pt) { return pt->xEnd; }
};

template <class DirectionWrapper>
void splitArea(int start, int end,
               QVector<const PointArea*> allPoints,
               QVector<MergedArea> &mergedAreas,
               const int otherStartLimit,
               const int otherEndLimit,
               DirectionWrapper w)
{
    MergedArea *currentArea = 0;

    for (int row = start; row <= end; row++) {
        QVector<const PointArea*> activePoints;

        QVector<const PointArea*>::iterator it = allPoints.begin();
        while (it != allPoints.end()) {
            const PointArea *pt = *it;

            if (w.mainEnd(pt) < row) {
                it = allPoints.erase(it);
                continue;
            }

            if (w.mainStart(pt) <= row) {
                activePoints.append(pt);
            }

            ++it;
        }

        if (activePoints.isEmpty()) continue;

        if (!currentArea || activePoints != currentArea->activePoints) {
            QVector<const PointArea*>::const_iterator it;

            int minOther = std::numeric_limits<int>::max();
            int maxOther = std::numeric_limits<int>::min();

            it = activePoints.constBegin();
            for (; it != activePoints.constEnd(); ++it) {
                const PointArea *pt = *it;

                writeIfMin(w.otherStart(pt), &minOther);
                writeIfMax(w.otherEnd(pt), &maxOther);
            }

            mergedAreas << MergedArea();
            currentArea = &mergedAreas.last();

            w.otherStart(currentArea) = qMax(minOther, otherStartLimit);
            w.otherEnd(currentArea) = qMin(maxOther, otherEndLimit);
            w.mainStart(currentArea) = row;
            w.mainEnd(currentArea) = row;

            currentArea->activePoints = activePoints;

        } else if (currentArea) {
            w.mainEnd(currentArea) = row;
        }
    }
}

void fillAreas(QVector<MergedArea> &mergedAreas)
{
/*    qDebug() << "= Fill";
    foreach (const KisMultipointPainter::MergedArea &a, mergedAreas) {
        qDebug() << "===";
        qDebug() << ppVar(a.xStart) << ppVar(a.xEnd);
        qDebug() << ppVar(a.yStart) << ppVar(a.yEnd);
        //qDebug() << ppVar(a.baseUnitValues.size());
        qDebug() << ppVar(a.dxValues.size());
        qDebug() << ppVar(a.dyValues.size());
        qDebug() << ppVar(a.activePoints.size());
    }
*/
    QVector<MergedArea>::iterator areaIt = mergedAreas.begin();
    for (; areaIt != mergedAreas.end(); ++areaIt) {
        MergedArea &area = *areaIt;

        const int width = area.xEnd - area.xStart + 1;
        const int height = area.yEnd - area.yStart + 1;
        area.dxValues.resize(width);
        area.dyValues.resize(height);
        area.baseUnitValue = 0.0;


        int ptIndex = 0;

        QVector<const PointArea*>::const_iterator it;
        it = area.activePoints.constBegin();

        for (; it != area.activePoints.constEnd(); ++it) {
            const PointArea *pt = *it;

            for (int x = area.xStart; x <= area.xEnd; x++) {
                //qDebug() << ptIndex << ppVar(x) << pt->dxValues[x - pt->xStart];

                area.dxValues[x - area.xStart] += pt->dxValues[x - pt->xStart];
            }

            for (int y = area.yStart; y <= area.yEnd; y++) {
                //qDebug() << ptIndex << ppVar(y) << pt->dyValues[y - pt->yStart];
                area.dyValues[y - area.yStart] += pt->dyValues[y - pt->yStart];
            }

            area.baseUnitValue += pt->baseUnitValue;
            ptIndex++;
        }

    }
}


void KisMultipointPainter::calcMergedAreas()
{
    QVector<const PointArea*> pointsCopy;
    foreach(const PointArea &pt, m_d->convertedPoints) {
        pointsCopy.append(&pt);
    }

    const int y0 = m_d->boundingRect.top();
    const int y1 = m_d->boundingRect.bottom();

    {
        const int otherStartLimit = std::numeric_limits<int>::min();
        const int otherEndLimit = std::numeric_limits<int>::max();
        splitArea(y0, y1,
                  pointsCopy, m_d->mergedAreas,
                  otherStartLimit,
                  otherEndLimit,
                  VerticalWrapper());
    }

    QVector<MergedArea> newMergedAreas;

    QVector<MergedArea>::const_iterator it = m_d->mergedAreas.constBegin();
    for (; it != m_d->mergedAreas.constEnd(); ++it) {
        const MergedArea &area = *it;

        {
            const int otherStartLimit = area.yStart;
            const int otherEndLimit = area.yEnd;
            splitArea(area.xStart, area.xEnd,
                      area.activePoints, newMergedAreas,
                      otherStartLimit,
                      otherEndLimit,
                      HorizontalWrapper());
        }
    }

    m_d->mergedAreas = newMergedAreas;

    fillAreas(m_d->mergedAreas);
}

QVector<KisMultipointPainter::MergedArea> KisMultipointPainter::fetchMergedAreas() const
{
    return m_d->mergedAreas;
}

void KisMultipointPainter::paintPoints3(KisPaintDeviceSP dev, const KoColor &color)
{
    const KoColorSpace *cs = dev->colorSpace();
    const quint32 pixelSize = cs->pixelSize();
    const quint8 *srcPtr = color.data();

    if (!m_d->dstIt) {
        m_d->dstIt = dev->createRandomAccessorNG(m_d->boundingRect.x(), m_d->boundingRect.y());
    }

    //int index = 0;

    foreach (const KisMultipointPainter::MergedArea &a, m_d->mergedAreas) {

        //qDebug() << "*** Area" << index++;

        for (int i = a.yStart; i <= a.yEnd; i++) {
            for (int j = a.xStart; j <= a.xEnd; j++) {

                float finalValue = 0.0;

                QVector<const PointArea*>::const_iterator it;
                it = a.activePoints.constBegin();

                for (; it != a.activePoints.constEnd(); ++it) {
                    const PointArea *pt = *it;

                    const float dx = pt->dxValues[j - pt->xStart];
                    const float dy = pt->dyValues[i - pt->yStart];
                    const float baseUnitValue = pt->baseUnitValue;

                    float value = baseUnitValue - dx - dy;

                    if (value > 0.0) {
                        finalValue += value;
                    } else {
                        continue;
                    }

                    if (finalValue > 1.0) {
                        break;
                    }
                }

                //qDebug() << j << i << ppVar(baseUnitValue) << ppVar(dx) << ppVar(dy) << ppVar(finalValue);

                if (finalValue >= 0.99) {
                    finalValue = 1.0;
                }

                if (finalValue > 0) {
                    m_d->dstIt->moveTo(j, i);
                    quint8 *dstPtr = m_d->dstIt->rawData();

                    float opacity = cs->opacityF(dstPtr);

                    if (opacity == 0.0) {
                        memcpy(dstPtr, srcPtr, pixelSize);
                    }

                    opacity += finalValue;
                    cs->setOpacity(dstPtr, opacity, 1);
                }

            }
        }
    }
}

void KisMultipointPainter::paintPoints2(KisPaintDeviceSP dev, const KoColor &color)
{
    const KoColorSpace *cs = dev->colorSpace();
    const quint32 pixelSize = cs->pixelSize();
    const quint8 *srcPtr = color.data();

    if (!m_d->dstIt) {
        m_d->dstIt = dev->createRandomAccessorNG(m_d->boundingRect.x(), m_d->boundingRect.y());
    }

    int index = 0;

    foreach (const KisMultipointPainter::MergedArea &a, m_d->mergedAreas) {

        //qDebug() << "*** Area" << index++;

        for (int i = a.yStart; i <= a.yEnd; i++) {
            const float dy = a.dyValues[i - a.yStart];

            for (int j = a.xStart; j <= a.xEnd; j++) {
                const float dx = a.dxValues[j - a.xStart];
                //const float baseUnitValue = a.baseUnitValues[j - a.xStart];
                const float baseUnitValue = a.baseUnitValue;

                float finalValue = baseUnitValue - dx - dy;

                qDebug() << j << i << ppVar(baseUnitValue) << ppVar(dx) << ppVar(dy) << ppVar(finalValue);

                if (finalValue >= 0.99) {
                    finalValue = 1.0;
                }

                if (finalValue > 0) {
                    m_d->dstIt->moveTo(j, i);
                    quint8 *dstPtr = m_d->dstIt->rawData();

                    float opacity = cs->opacityF(dstPtr);

                    if (opacity == 0.0) {
                        memcpy(dstPtr, srcPtr, pixelSize);
                    }

                    opacity += finalValue;
                    cs->setOpacity(dstPtr, opacity, 1);
                }

            }
        }
    }
}

void KisMultipointPainter::paintPoints(KisPaintDeviceSP dev, const KoColor &color)
{
    const KoColorSpace *cs = dev->colorSpace();
    const quint32 pixelSize = cs->pixelSize();
    const quint8 *srcPtr = color.data();

    if (!m_d->dstIt) {
        m_d->dstIt = dev->createRandomAccessorNG(m_d->boundingRect.x(), m_d->boundingRect.y());
    }

    const int y0 = m_d->boundingRect.top();
    const int y1 = m_d->boundingRect.bottom();

    for (int y = y0; y <= y1; y++) {
        CompositeRow row = getCompositeRow(y);

        int x = row.start;

        while (x <= row.end) {
            float finalValue = 0.0;

            for (int i = 0; i < row.rows.size(); i++) {
                const PointRow &pt = row.rows[i];

                if (pt.start <= x && x <= pt.end) {
                    const float dx = (*pt.dxValues)[x - pt.start];
                    const float dy = pt.dyValue;

                    float value = pt.baseUnitValue - dx - dy;

                    if (value <= 0.0) {
                        continue;
                    } else {
                        //finalValue += (1.0 - finalValue) * value;
                        finalValue += value;
                        //finalValue += (pt.baseUnitValue - finalValue) * value;
                        //finalValue = qMin(pt.baseUnitValue, value);
                    }

                    if (finalValue >= 0.99) {
                        finalValue = 1.0;
                        break;
                    }
                }
            }

            if (finalValue > 0) {
                m_d->dstIt->moveTo(x, y);
                quint8 *dstPtr = m_d->dstIt->rawData();

                float opacity = cs->opacityF(dstPtr);

                if (opacity == 0.0) {
                    memcpy(dstPtr, srcPtr, pixelSize);
                }

                //opacity += (1.0 - opacity) * finalValue;
                opacity += finalValue;
                //opacity = qMax(finalValue, opacity);

                cs->setOpacity(dstPtr, opacity, 1);
            }

            if (finalValue > 1.9999) {
                 qDebug() << ppVar(finalValue);
            }

/*            if (finalValue > 0) {
                m_d->dstIt->moveTo(x, y);
                quint8 *dstPtr = m_d->dstIt->rawData();

                float opacity = cs->opacityF(dstPtr);

                if (opacity == 0.0) {
                    memcpy(dstPtr, srcPtr, pixelSize);
                }

                if (finalValue > opacity) {
                    opacity += (1.0 - opacity) * finalValue;
                    cs->setOpacity(dstPtr, opacity, 1);
                }
                }*/
            x++;
        }
    }
}


