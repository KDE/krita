/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisWatershedWorker.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoAlwaysInline.h>

#include "kis_lazy_fill_tools.h"

#include "kis_paint_device_debug_utils.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_sequential_iterator.h"
#include "kis_scanline_fill.h"

#include "kis_random_accessor_ng.h"

#include <boost/heap/fibonacci_heap.hpp>

using namespace KisLazyFillTools;

namespace {
struct FillGroup {
    FillGroup() {}
    FillGroup(int _colorIndex) : colorIndex(_colorIndex) {}

    int colorIndex = -1;

    struct LevelData {
        int positiveEdgeSize = 0;
        int negativeEdgeSize = 0;
        int foreignEdgeSize = 0;
        int openEdgeSize = 0;

        int lastDistance = 0;
    };

    QMap<int, LevelData> levels;
};

enum PrevDirections
{
    FROM_NOWHERE = 0,
    FROM_RIGHT,
    FROM_LEFT,
    FROM_TOP,
    FROM_BOTTOM
};

struct TaskPoint {
    int x = 0;
    int y = 0;
    int distance = 0;
    qint32 group = 0;
    quint8 prevDirection = 0;
    quint8 level = 0;
};

struct CompareTaskPoints {
    bool operator()(const TaskPoint &pt1, const TaskPoint &pt2) const {
        return
            pt1.level > pt2.level || (pt1.level == pt2.level && pt1.distance > pt2.distance);
    }
};

/**
 * Adjusts the stroke device in a way that all the stroke's pixels
 * are set to the range 1...255, according to the height of this pixel
 * in the heightmap. The pixels not having any stroke have value 0
 */
void mergeHeightmapOntoStroke(KisPaintDeviceSP stroke, KisPaintDeviceSP heightMap, const QRect &rc)
{
    KisSequentialIterator dstIt(stroke, rc);
    KisSequentialIterator mapIt(heightMap, rc);

    do {
        quint8 *dstPtr = dstIt.rawData();

        if (*dstPtr > 0) {
            quint8 *mapPtr = mapIt.rawData();
            *dstPtr = qMax(quint8(1), *mapPtr);
        } else {
            *dstPtr = 0;
        }

    } while (dstIt.nextPixel() && mapIt.nextPixel());
}

void parseColorIntoGroups(QVector<FillGroup> &groups,
                          KisPaintDeviceSP groupMap,
                          KisPaintDeviceSP heightMap,
                          int colorIndex,
                          KisPaintDeviceSP stroke,
                          const QRect &boundingRect)
{
    const QRect strokeRect = stroke->exactBounds();
    mergeHeightmapOntoStroke(stroke, heightMap, strokeRect);

    KisSequentialIterator dstIt(stroke, strokeRect);

    do {
        quint8 *dstPtr = dstIt.rawData();

        if (*dstPtr > 0) {
            const QPoint pt(dstIt.x(), dstIt.y());
            KisScanlineFill fill(stroke, pt, boundingRect);
            /**
             * The threshold is set explicitly. If you want to raise it,
             * don't forget to add a destiction between 0 and >0 in
             * the fill strategy. Otherwise the algorithm will not work.
             */
            fill.setThreshold(0);
            fill.fillContiguousGroup(groupMap, groups.size());

            groups << FillGroup(colorIndex);
        }

    } while (dstIt.nextPixel());
}

using PointsPriorityQueue = boost::heap::fibonacci_heap<TaskPoint, boost::heap::compare<CompareTaskPoints>>;

}

/***********************************************************************/
/*           KisWatershedWorker::Private                               */
/***********************************************************************/

struct KisWatershedWorker::Private
{
    Private() : pointsQueue(pointsComparator) {}

    KisPaintDeviceSP heightMap;
    KisPaintDeviceSP dstDevice;

    QRect boundingRect;
    QVector<KeyStroke> keyStrokes;

    QVector<FillGroup> groups;
    KisPaintDeviceSP groupsMap;

    CompareTaskPoints pointsComparator;
    PointsPriorityQueue pointsQueue;

    // temporary "global" variables for the processing routines
    KisRandomAccessorSP groupIt;
    KisRandomConstAccessorSP levelIt;

    void initializeQueueFromGroupMap(const QRect &rc);

    inline void visitNeighbour(int x, int y, quint8 fromDirection, int prevDistance, qint32 prevGroup, quint8 prevLevel, FillGroup::LevelData &prevLevelData);
    inline void updateGroupLastDistance(FillGroup::LevelData &levelData, int distance);
    void processQueue();
    void writeColoring();

    void dumpGroupMaps();


};

/***********************************************************************/
/*           KisWatershedWorker                                        */
/***********************************************************************/



KisWatershedWorker::KisWatershedWorker(KisPaintDeviceSP heightMap, KisPaintDeviceSP dst, const QRect &boundingRect)
    : m_d(new Private)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(heightMap->colorSpace()->pixelSize() == 1);

    m_d->heightMap = heightMap;
    m_d->dstDevice = dst;
    m_d->boundingRect = boundingRect;

    // Just the simplest color space with 4 bytes per pixel. We use it as
    // a storage for qint32-indexed group ids
    m_d->groupsMap = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
}

KisWatershedWorker::~KisWatershedWorker()
{
}

void KisWatershedWorker::addKeyStroke(KisPaintDeviceSP dev, const KoColor &color)
{

    // TODO: make it clear about mutability of the stroke devices
    m_d->keyStrokes << KeyStroke(dev, color);
}

void KisWatershedWorker::run()
{
    if (!m_d->heightMap) return;

    m_d->groups << FillGroup(-1);

    for (int i = 0; i < m_d->keyStrokes.size(); i++) {
        parseColorIntoGroups(m_d->groups, m_d->groupsMap,
                             m_d->heightMap,
                             i, m_d->keyStrokes[i].dev,
                             m_d->boundingRect);
    }

    m_d->dumpGroupMaps();

    const QRect initRect =
        m_d->boundingRect & m_d->groupsMap->nonDefaultPixelArea();

    m_d->initializeQueueFromGroupMap(initRect);

    m_d->processQueue();

    m_d->dumpGroupMaps();

    m_d->writeColoring();
}


void KisWatershedWorker::Private::initializeQueueFromGroupMap(const QRect &rc)
{
    KisSequentialConstIterator groupMapIt(groupsMap, rc);
    KisSequentialConstIterator heightMapIt(heightMap, rc);

    do {
        const qint32 *groupPtr = reinterpret_cast<const qint32*>(groupMapIt.rawDataConst());
        const quint8 *heightPtr = heightMapIt.rawDataConst();

        if (*groupPtr > 0) {
            TaskPoint pt;
            pt.x = groupMapIt.x();
            pt.y = groupMapIt.y();
            pt.group = *groupPtr;
            pt.level = *heightPtr;

            pointsQueue.push(pt);
        }

    } while (groupMapIt.nextPixel() &&
             heightMapIt.nextPixel());
}

void KisWatershedWorker::Private::visitNeighbour(int x, int y,
                                                 quint8 fromDirection,
                                                 int prevDistance,
                                                 qint32 prevGroup,
                                                 quint8 prevLevel,
                                                 FillGroup::LevelData &prevLevelData)
{
    if (!boundingRect.contains(x, y)) {
        prevLevelData.positiveEdgeSize++;
        return;
    }

    groupIt->moveTo(x, y);
    levelIt->moveTo(x, y);

    const qint32 *groupPtr = reinterpret_cast<const qint32*>(groupIt->rawDataConst());
    const quint8 newLevel = *levelIt->rawDataConst();


    const qint32 currentGroup = *groupPtr;

    if (!currentGroup || currentGroup == prevGroup) {

        // adjust the group metrics
        if (newLevel == prevLevel) {
            // noop
        } else if (newLevel > prevLevel) {
            FillGroup::LevelData &currLevelData = groups[currentGroup].levels[newLevel];

            prevLevelData.positiveEdgeSize++;
            currLevelData.negativeEdgeSize++;
        } else if (newLevel < prevLevel) {
            FillGroup::LevelData &currLevelData = groups[currentGroup].levels[newLevel];

            prevLevelData.negativeEdgeSize++;
            currLevelData.positiveEdgeSize++;
        }

        // append fill task
        if (!currentGroup) {
            TaskPoint pt;
            pt.x = x;
            pt.y = y;
            pt.group = prevGroup;
            pt.level = newLevel;
            pt.distance = prevDistance + 1;
            pt.prevDirection = fromDirection;
            pointsQueue.push(pt);
        }

    } else {
        // foreign pixels are the priority
        prevLevelData.foreignEdgeSize++;
        groups[*groupPtr].levels[newLevel].foreignEdgeSize++;
    }
}

void KisWatershedWorker::Private::updateGroupLastDistance(FillGroup::LevelData &levelData, int distance)
{
    if (levelData.lastDistance > distance) {
        qDebug() << ppVar(levelData.lastDistance)  << ppVar(distance);
    }

    levelData.lastDistance = distance;
}

void KisWatershedWorker::Private::processQueue()
{
    // TODO: lazy initialization of the iterator's position
    groupIt = groupsMap->createRandomAccessorNG(boundingRect.x(), boundingRect.y());
    levelIt = heightMap->createRandomConstAccessorNG(boundingRect.x(), boundingRect.y());

    while (!pointsQueue.empty()) {
        TaskPoint pt = pointsQueue.top();
        pointsQueue.pop();

        groupIt->moveTo(pt.x, pt.y);
        qint32 *groupPtr = reinterpret_cast<qint32*>(groupIt->rawData());

        FillGroup::LevelData &ptLevelData = groups[pt.group].levels[pt.level];

        //ENTER_FUNCTION() << pt.x << pt.y << ppVar(pt.group) << ppVar(pt.level) << ppVar(pt.distance) << ppVar(*groupPtr);

        if (*groupPtr == 0 || pt.prevDirection == FROM_NOWHERE) {

            switch (pt.prevDirection) {
            case FROM_NOWHERE:
                visitNeighbour(pt.x - 1, pt.y, FROM_RIGHT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x + 1, pt.y, FROM_LEFT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y - 1, FROM_BOTTOM, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y + 1, FROM_TOP, pt.distance, pt.group, pt.level, ptLevelData);

                break;
            case FROM_LEFT:
                visitNeighbour(pt.x + 1, pt.y, FROM_LEFT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y - 1, FROM_BOTTOM, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y + 1, FROM_TOP, pt.distance, pt.group, pt.level, ptLevelData);
                *groupPtr = pt.group;
                updateGroupLastDistance(ptLevelData, pt.distance);

                break;
            case FROM_RIGHT:
                visitNeighbour(pt.x - 1, pt.y, FROM_RIGHT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y - 1, FROM_BOTTOM, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y + 1, FROM_TOP, pt.distance, pt.group, pt.level, ptLevelData);
                *groupPtr = pt.group;
                updateGroupLastDistance(ptLevelData, pt.distance);

                break;
            case FROM_TOP:
                visitNeighbour(pt.x - 1, pt.y, FROM_RIGHT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x + 1, pt.y, FROM_LEFT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y + 1, FROM_TOP, pt.distance, pt.group, pt.level, ptLevelData);
                *groupPtr = pt.group;
                updateGroupLastDistance(ptLevelData, pt.distance);

                break;
            case FROM_BOTTOM:
                visitNeighbour(pt.x - 1, pt.y, FROM_RIGHT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x + 1, pt.y, FROM_LEFT, pt.distance, pt.group, pt.level, ptLevelData);
                visitNeighbour(pt.x, pt.y - 1, FROM_BOTTOM, pt.distance, pt.group, pt.level, ptLevelData);
                *groupPtr = pt.group;
                updateGroupLastDistance(ptLevelData, pt.distance);
                break;
            }

        } else {
            // nothing to do?
        }

    }

    // cleaup iterators
    groupIt.clear();
    levelIt.clear();
}

void KisWatershedWorker::Private::writeColoring()
{
    KisSequentialConstIterator srcIt(groupsMap, boundingRect);
    KisSequentialIterator dstIt(dstDevice, boundingRect);

    QVector<KoColor> colors;
    for (auto it = keyStrokes.begin(); it != keyStrokes.end(); ++it) {
        KoColor color = it->color;
        color.convertTo(dstDevice->colorSpace());
        colors << color;
    }
    const int colorPixelSize = dstDevice->pixelSize();


    do {
        const qint32 *srcPtr = reinterpret_cast<const qint32*>(srcIt.rawDataConst());

        const int colorIndex = groups[*srcPtr].colorIndex;
        if (colorIndex >= 0) {
            memcpy(dstIt.rawData(), colors[colorIndex].data(), colorPixelSize);
        }

    } while (srcIt.nextPixel() && dstIt.nextPixel());
}

void KisWatershedWorker::Private::dumpGroupMaps()
{
    KisPaintDeviceSP groupDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP colorDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP pedgeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP nedgeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP fedgeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    KisSequentialConstIterator srcIt(groupsMap, boundingRect);
    KisSequentialIterator dstGroupIt(groupDevice, boundingRect);
    KisSequentialIterator dstColorIt(colorDevice, boundingRect);
    KisSequentialIterator dstPedgeIt(pedgeDevice, boundingRect);
    KisSequentialIterator dstNedgeIt(nedgeDevice, boundingRect);
    KisSequentialIterator dstFedgeIt(fedgeDevice, boundingRect);


    QVector<KoColor> colors;
    for (auto it = keyStrokes.begin(); it != keyStrokes.end(); ++it) {
        KoColor color = it->color;
        color.convertTo(colorDevice->colorSpace());
        colors << color;
    }
    const int colorPixelSize = colorDevice->pixelSize();



    do {
        const qint32 *srcPtr = reinterpret_cast<const qint32*>(srcIt.rawDataConst());

        *dstGroupIt.rawData() = quint8(*srcPtr);
        memcpy(dstColorIt.rawData(), colors[groups[*srcPtr].colorIndex].data(), colorPixelSize);

        const FillGroup::LevelData &l = groups[*srcPtr].levels[0];

        *dstPedgeIt.rawData() = 255.0 * qreal(l.positiveEdgeSize) / (l.positiveEdgeSize + l.negativeEdgeSize + l.foreignEdgeSize);
        *dstNedgeIt.rawData() = 255.0 * qreal(l.negativeEdgeSize) / (l.positiveEdgeSize + l.negativeEdgeSize + l.foreignEdgeSize);
        *dstFedgeIt.rawData() = 255.0 * qreal(l.foreignEdgeSize) / (l.positiveEdgeSize + l.negativeEdgeSize + l.foreignEdgeSize);

    } while (dstGroupIt.nextPixel() &&
             srcIt.nextPixel() &&
             dstColorIt.nextPixel() &&
             dstPedgeIt.nextPixel() &&
             dstNedgeIt.nextPixel() &&
             dstFedgeIt.nextPixel());


    KIS_DUMP_DEVICE_2(groupDevice, boundingRect, "01_groupMap", "dd");
    KIS_DUMP_DEVICE_2(colorDevice, boundingRect, "02_colorMap", "dd");
    KIS_DUMP_DEVICE_2(pedgeDevice, boundingRect, "03_pedgeMap", "dd");
    KIS_DUMP_DEVICE_2(nedgeDevice, boundingRect, "04_nedgeMap", "dd");
    KIS_DUMP_DEVICE_2(fedgeDevice, boundingRect, "05_fedgeMap", "dd");
}
