/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWatershedWorker.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoAlwaysInline.h>
#include <KoUpdater.h>

#include "kis_lazy_fill_tools.h"

#include "kis_paint_device_debug_utils.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_sequential_iterator.h"
#include "kis_scanline_fill.h"

#include "kis_random_accessor_ng.h"

#include <boost/heap/fibonacci_heap.hpp>
#include <set>

using namespace KisLazyFillTools;

namespace {

struct CompareQPoints
{
    bool operator() (const QPoint &p1, const QPoint &p2) const {
        return p1.y() < p2.y() || (p1.y() == p2.y() && p1.x() < p2.x());
    }
};

struct FillGroup {
    FillGroup() {}
    FillGroup(int _colorIndex) : colorIndex(_colorIndex) {}

    int colorIndex = -1;

    struct LevelData {
        int positiveEdgeSize = 0;
        int negativeEdgeSize = 0;
        int foreignEdgeSize = 0;
        int allyEdgeSize = 0;
        int numFilledPixels = 0;

        bool narrowRegion = false;

        int totalEdgeSize() const {
            return positiveEdgeSize + negativeEdgeSize + foreignEdgeSize + allyEdgeSize;
        }

        QMap<qint32, std::multiset<QPoint, CompareQPoints>> conflictWithGroup;
    };

    QMap<int, LevelData> levels;
};

using GroupLevelPair = QPair<qint32, quint8>;

enum PrevDirections
{
    FROM_NOWHERE = 0,
    FROM_RIGHT,
    FROM_LEFT,
    FROM_TOP,
    FROM_BOTTOM
};

struct NeighbourStaticOffset
{
    const quint8 from;
    const bool statsOnly;
    const QPoint offset;
};

static NeighbourStaticOffset staticOffsets[5][4] =
{
    { // FROM_NOWHERE
        { FROM_RIGHT,  false, QPoint(-1,  0) },
        { FROM_LEFT,   false, QPoint( 1,  0) },
        { FROM_BOTTOM, false, QPoint( 0, -1) },
        { FROM_TOP,    false, QPoint( 0,  1) },
    },
    { // FROM_RIGHT
        { FROM_RIGHT,  false, QPoint(-1,  0) },
        { FROM_LEFT,   true,  QPoint( 1,  0) },
        { FROM_BOTTOM, false, QPoint( 0, -1) },
        { FROM_TOP,    false, QPoint( 0,  1) },
    },
    { // FROM_LEFT
        { FROM_RIGHT,  true,  QPoint(-1,  0) },
        { FROM_LEFT,   false, QPoint( 1,  0) },
        { FROM_BOTTOM, false, QPoint( 0, -1) },
        { FROM_TOP,    false, QPoint( 0,  1) },
    },
    { // FROM_TOP
        { FROM_RIGHT,  false, QPoint(-1,  0) },
        { FROM_LEFT,   false, QPoint( 1,  0) },
        { FROM_BOTTOM, true,  QPoint( 0, -1) },
        { FROM_TOP,    false, QPoint( 0,  1) },
    },
    { // FROM_BOTTOM
        { FROM_RIGHT,  false, QPoint(-1,  0) },
        { FROM_LEFT,   false, QPoint( 1,  0) },
        { FROM_BOTTOM, false, QPoint( 0, -1) },
        { FROM_TOP,    true,  QPoint( 0,  1) },
    }
};

struct TaskPoint {
    int x = 0;
    int y = 0;
    int distance = 0;
    qint32 group = 0;
    quint8 prevDirection = FROM_NOWHERE;
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
    KisSequentialConstIterator mapIt(heightMap, rc);

    while (dstIt.nextPixel() && mapIt.nextPixel()) {
        quint8 *dstPtr = dstIt.rawData();

        if (*dstPtr > 0) {
            const quint8 *mapPtr = mapIt.rawDataConst();
            *dstPtr = qMax(quint8(1), *mapPtr);
        } else {
            *dstPtr = 0;
        }

    }
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

    while (dstIt.nextPixel()) {
        quint8 *dstPtr = dstIt.rawData();

        if (*dstPtr > 0) {
            const QPoint pt(dstIt.x(), dstIt.y());
            KisScanlineFill fill(stroke, pt, boundingRect);
            /**
             * The threshold is set explicitly. If you want to raise it,
             * don't forget to add a distinction between 0 and >0 in
             * the fill strategy. Otherwise the algorithm will not work.
             */
            fill.setThreshold(0);
            fill.fillContiguousGroup(groupMap, groups.size());

            groups << FillGroup(colorIndex);
        }

    }
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
    qint32 backgroundGroupId = 0;
    int backgroundGroupColor = -1;
    bool recolorMode = false;

    quint64 totalPixelsToFill = 0;
    quint64 numFilledPixels = 0;

    KoUpdater *progressUpdater = 0;

    void initializeQueueFromGroupMap(const QRect &rc);

    ALWAYS_INLINE void visitNeighbour(const QPoint &currPt, const QPoint &prevPt, quint8 fromDirection, int prevDistance, quint8 prevLevel, qint32 prevGroupId, FillGroup &prevGroup, FillGroup::LevelData &prevLevelData, qint32 prevPrevGroupId, FillGroup &prevPrevGroup, bool statsOnly = false);
    ALWAYS_INLINE void updateGroupLastDistance(FillGroup::LevelData &levelData, int distance);
    void processQueue(qint32 _backgroundGroupId);
    void writeColoring();

    QVector<TaskPoint> tryRemoveConflictingPlane(qint32 group, quint8 level);

    void updateNarrowRegionMetrics();

    QVector<GroupLevelPair> calculateConflictingPairs();
    void cleanupForeignEdgeGroups(qreal cleanUpAmount);

    void dumpGroupMaps();
    void calcNumGroupMaps();
    void dumpGroupInfo(qint32 groupIndex, quint8 levelIndex);


};

/***********************************************************************/
/*           KisWatershedWorker                                        */
/***********************************************************************/



KisWatershedWorker::KisWatershedWorker(KisPaintDeviceSP heightMap, KisPaintDeviceSP dst, const QRect &boundingRect, KoUpdater *progress)
    : m_d(new Private)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(heightMap->colorSpace()->pixelSize() == 1);

    m_d->progressUpdater = progress;
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
    m_d->keyStrokes << KeyStroke(new KisPaintDevice(*dev), color);

    KisPaintDeviceSP lastDev = m_d->keyStrokes.back().dev;

    for (auto it = m_d->keyStrokes.begin(); it != m_d->keyStrokes.end() - 1; ++it) {
        KisPaintDeviceSP dev = it->dev;
        const QRect rc = dev->exactBounds() & lastDev->exactBounds();
        if (rc.isEmpty()) continue;

        KisSequentialIterator devIt(dev, rc);
        KisSequentialConstIterator lastDevIt(lastDev, rc);

        while (devIt.nextPixel() &&
               lastDevIt.nextPixel()) {

            quint8 *devPtr = devIt.rawData();
            const quint8 *lastDevPtr = lastDevIt.rawDataConst();

            if (*devPtr > 0 && *lastDevPtr > 0) {
                *devPtr = 0;
            }

        }
    }
}

void KisWatershedWorker::run(qreal cleanUpAmount)
{
    if (!m_d->heightMap) return;

    m_d->groups << FillGroup(-1);

    for (int i = 0; i < m_d->keyStrokes.size(); i++) {
        parseColorIntoGroups(m_d->groups, m_d->groupsMap,
                             m_d->heightMap,
                             i, m_d->keyStrokes[i].dev,
                             m_d->boundingRect);
    }

//    m_d->dumpGroupMaps();
//    m_d->calcNumGroupMaps();

    const QRect initRect =
        m_d->boundingRect & m_d->groupsMap->nonDefaultPixelArea();

    m_d->initializeQueueFromGroupMap(initRect);
    m_d->processQueue(0);

//    m_d->dumpGroupMaps();
//    m_d->calcNumGroupMaps();

    if (cleanUpAmount > 0) {
        m_d->cleanupForeignEdgeGroups(cleanUpAmount);
    }

//    m_d->calcNumGroupMaps();

    m_d->writeColoring();

}

int KisWatershedWorker::testingGroupPositiveEdge(qint32 group, quint8 level)
{
    return m_d->groups[group].levels[level].positiveEdgeSize;
}

int KisWatershedWorker::testingGroupNegativeEdge(qint32 group, quint8 level)
{
    return m_d->groups[group].levels[level].negativeEdgeSize;
}

int KisWatershedWorker::testingGroupForeignEdge(qint32 group, quint8 level)
{
    return m_d->groups[group].levels[level].foreignEdgeSize;
}

int KisWatershedWorker::testingGroupAllyEdge(qint32 group, quint8 level)
{
    return m_d->groups[group].levels[level].allyEdgeSize;
}

int KisWatershedWorker::testingGroupConflicts(qint32 group, quint8 level, qint32 withGroup)
{
    return m_d->groups[group].levels[level].conflictWithGroup[withGroup].size();
}

void KisWatershedWorker::testingTryRemoveGroup(qint32 group, quint8 levelIndex)
{
    QVector<TaskPoint> taskPoints =
        m_d->tryRemoveConflictingPlane(group, levelIndex);

    if (!taskPoints.isEmpty()) {
        Q_FOREACH (const TaskPoint &pt, taskPoints) {
            m_d->pointsQueue.push(pt);
        }
        m_d->processQueue(group);
    }
    m_d->dumpGroupMaps();
    m_d->calcNumGroupMaps();
}

void KisWatershedWorker::Private::initializeQueueFromGroupMap(const QRect &rc)
{
    KisSequentialIterator groupMapIt(groupsMap, rc);
    KisSequentialConstIterator heightMapIt(heightMap, rc);

    while (groupMapIt.nextPixel() &&
           heightMapIt.nextPixel()) {

        qint32 *groupPtr = reinterpret_cast<qint32*>(groupMapIt.rawData());
        const quint8 *heightPtr = heightMapIt.rawDataConst();

        if (*groupPtr > 0) {
            TaskPoint pt;
            pt.x = groupMapIt.x();
            pt.y = groupMapIt.y();
            pt.group = *groupPtr;
            pt.level = *heightPtr;

            pointsQueue.push(pt);

            // we must clear the pixel to make sure foreign metric is calculated correctly
            *groupPtr = 0;
        }

    }
}

ALWAYS_INLINE void addForeignAlly(qint32 currGroupId,
                                  qint32 prevGroupId,
                                  FillGroup &currGroup,
                                  FillGroup &prevGroup,
                                  FillGroup::LevelData &currLevelData,
                                  FillGroup::LevelData &prevLevelData,
                                  const QPoint &currPt, const QPoint &prevPt,
                                  bool sameLevel)
{
    if (currGroup.colorIndex != prevGroup.colorIndex || !sameLevel) {
        prevLevelData.foreignEdgeSize++;
        currLevelData.foreignEdgeSize++;

        if (sameLevel) {
            currLevelData.conflictWithGroup[prevGroupId].insert(currPt);
            prevLevelData.conflictWithGroup[currGroupId].insert(prevPt);
        }

    } else {
        prevLevelData.allyEdgeSize++;
        currLevelData.allyEdgeSize++;
    }


}

ALWAYS_INLINE  void removeForeignAlly(qint32 currGroupId,
                                      qint32 prevGroupId,
                                      FillGroup &currGroup,
                                      FillGroup &prevGroup,
                                      FillGroup::LevelData &currLevelData,
                                      FillGroup::LevelData &prevLevelData,
                                      const QPoint &currPt, const QPoint &prevPt,
                                      bool sameLevel)
{
    if (currGroup.colorIndex != prevGroup.colorIndex || !sameLevel) {
        prevLevelData.foreignEdgeSize--;
        currLevelData.foreignEdgeSize--;

        if (sameLevel) {
            std::multiset<QPoint, CompareQPoints> &currSet = currLevelData.conflictWithGroup[prevGroupId];
            currSet.erase(currSet.find(currPt));

            std::multiset<QPoint, CompareQPoints> &prevSet = prevLevelData.conflictWithGroup[currGroupId];
            prevSet.erase(prevSet.find(prevPt));
        }

    } else {
        prevLevelData.allyEdgeSize--;
        currLevelData.allyEdgeSize--;
    }


}

ALWAYS_INLINE  void incrementLevelEdge(FillGroup::LevelData &currLevelData,
                                       FillGroup::LevelData &prevLevelData,
                                       quint8 currLevel,
                                       quint8 prevLevel)
{
    Q_ASSERT(currLevel != prevLevel);

    if (currLevel > prevLevel) {
        currLevelData.negativeEdgeSize++;
        prevLevelData.positiveEdgeSize++;
    } else {
        currLevelData.positiveEdgeSize++;
        prevLevelData.negativeEdgeSize++;
    }
}

ALWAYS_INLINE  void decrementLevelEdge(FillGroup::LevelData &currLevelData,
                                       FillGroup::LevelData &prevLevelData,
                                       quint8 currLevel,
                                       quint8 prevLevel)
{
    Q_ASSERT(currLevel != prevLevel);

    if (currLevel > prevLevel) {
        currLevelData.negativeEdgeSize--;
        prevLevelData.positiveEdgeSize--;
    } else {
        currLevelData.positiveEdgeSize--;
        prevLevelData.negativeEdgeSize--;
    }
}

void KisWatershedWorker::Private::visitNeighbour(const QPoint &currPt, const QPoint &prevPt,
                                                 quint8 fromDirection, int prevDistance, quint8 prevLevel,
                                                 qint32 prevGroupId, FillGroup &prevGroup, FillGroup::LevelData &prevLevelData,
                                                 qint32 prevPrevGroupId, FillGroup &prevPrevGroup,
                                                 bool statsOnly)
{
    if (!boundingRect.contains(currPt)) {
        prevLevelData.positiveEdgeSize++;

        if (prevPrevGroupId > 0) {
            FillGroup::LevelData &prevPrevLevelData = prevPrevGroup.levels[prevLevel];
            prevPrevLevelData.positiveEdgeSize--;
        }
        return;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(prevGroupId != backgroundGroupId);

    groupIt->moveTo(currPt.x(), currPt.y());
    levelIt->moveTo(currPt.x(), currPt.y());

    const qint32 currGroupId = *reinterpret_cast<const qint32*>(groupIt->rawDataConst());
    const quint8 newLevel = *levelIt->rawDataConst();

    FillGroup &currGroup = groups[currGroupId];
    FillGroup::LevelData &currLevelData = currGroup.levels[newLevel];

    const bool needsAddTaskPoint =
        !currGroupId ||
        (recolorMode &&
         ((newLevel == prevLevel &&
           currGroupId == backgroundGroupId) ||
          (newLevel >= prevLevel &&
           currGroup.colorIndex == backgroundGroupColor &&
           currLevelData.narrowRegion)));

    if (needsAddTaskPoint && !statsOnly) {
        TaskPoint pt;
        pt.x = currPt.x();
        pt.y = currPt.y();
        pt.group = prevGroupId;
        pt.level = newLevel;
        pt.distance = newLevel == prevLevel ? prevDistance + 1 : 0;
        pt.prevDirection = fromDirection;

        pointsQueue.push(pt);
    }

    // we can never clear the pixel!
    KIS_SAFE_ASSERT_RECOVER_RETURN(prevGroupId > 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN(prevGroupId != prevPrevGroupId);

    if (currGroupId) {
        const bool isSameLevel = prevLevel == newLevel;


        if ((!prevPrevGroupId ||
             prevPrevGroupId == currGroupId) &&
            prevGroupId != currGroupId) {

            // we have added a foreign/ally group

            FillGroup::LevelData &currLevelData = currGroup.levels[newLevel];

            addForeignAlly(currGroupId, prevGroupId,
                           currGroup, prevGroup,
                           currLevelData, prevLevelData,
                           currPt, prevPt,
                           isSameLevel);

        } else if (prevPrevGroupId &&
                   prevPrevGroupId != currGroupId &&
                   prevGroupId == currGroupId) {

            // we have removed a foreign/ally group

            FillGroup::LevelData &currLevelData = currGroup.levels[newLevel];
            FillGroup::LevelData &prevPrevLevelData = prevPrevGroup.levels[prevLevel];

            removeForeignAlly(currGroupId, prevPrevGroupId,
                              currGroup, prevPrevGroup,
                              currLevelData, prevPrevLevelData,
                              currPt, prevPt,
                              isSameLevel);

        } else if (prevPrevGroupId &&
                   prevPrevGroupId != currGroupId &&
                   prevGroupId != currGroupId) {

            // this pixel has become an foreign/ally pixel of a different group

            FillGroup::LevelData &currLevelData = currGroup.levels[newLevel];

            FillGroup &prevPrevGroup = groups[prevPrevGroupId];
            FillGroup::LevelData &prevPrevLevelData = prevPrevGroup.levels[prevLevel];

            removeForeignAlly(currGroupId, prevPrevGroupId,
                              currGroup, prevPrevGroup,
                              currLevelData, prevPrevLevelData,
                              currPt, prevPt,
                              isSameLevel);

            addForeignAlly(currGroupId, prevGroupId,
                           currGroup, prevGroup,
                           currLevelData, prevLevelData,
                           currPt, prevPt,
                           isSameLevel);
        }

        if (!isSameLevel) {

            if (prevGroupId == currGroupId) {
                // we connected with our own disjoint area

                FillGroup::LevelData &currLevelData = currGroup.levels[newLevel];

                incrementLevelEdge(currLevelData, prevLevelData,
                                   newLevel, prevLevel);
            }

            if (prevPrevGroupId == currGroupId) {
                // we removed a pixel for the borderline
                // (now it is registered as foreign/ally pixel)

                FillGroup::LevelData &currLevelData = currGroup.levels[newLevel];
                FillGroup::LevelData &prevPrevLevelData = currGroup.levels[prevLevel];

                decrementLevelEdge(currLevelData, prevPrevLevelData,
                                   newLevel, prevLevel);
            }
        }
    }
}

#include <QElapsedTimer>

void KisWatershedWorker::Private::processQueue(qint32 _backgroundGroupId)
{
    QElapsedTimer tt; tt.start();


    // TODO: lazy initialization of the iterator's position
    // TODO: reuse iterators if possible!
    groupIt = groupsMap->createRandomAccessorNG();
    levelIt = heightMap->createRandomConstAccessorNG();
    backgroundGroupId = _backgroundGroupId;
    backgroundGroupColor = groups[backgroundGroupId].colorIndex;
    recolorMode = backgroundGroupId > 1;

    totalPixelsToFill = qint64(boundingRect.width()) * boundingRect.height();
    numFilledPixels = 0;
    const int progressReportingMask = (1 << 18) - 1; // report every 512x512 patch


    if (recolorMode) {
        updateNarrowRegionMetrics();
    }

    while (!pointsQueue.empty()) {
        TaskPoint pt = pointsQueue.top();
        pointsQueue.pop();

        groupIt->moveTo(pt.x, pt.y);
        qint32 *groupPtr = reinterpret_cast<qint32*>(groupIt->rawData());

        const qint32 prevGroupId = *groupPtr;
        FillGroup &prevGroup = groups[prevGroupId];

        if (prevGroupId == backgroundGroupId ||
            (recolorMode &&
             prevGroup.colorIndex == backgroundGroupColor)) {

            FillGroup &currGroup = groups[pt.group];
            FillGroup::LevelData &currLevelData = currGroup.levels[pt.level];
            currLevelData.numFilledPixels++;

            if (prevGroupId > 0) {
                FillGroup::LevelData &prevLevelData = prevGroup.levels[pt.level];
                prevLevelData.numFilledPixels--;
            } else {
                numFilledPixels++;
            }

            const NeighbourStaticOffset *offsets = staticOffsets[pt.prevDirection];
            const QPoint currPt(pt.x, pt.y);

            for (int i = 0; i < 4; i++) {
                const NeighbourStaticOffset &offset = offsets[i];

                const QPoint nextPt = currPt + offset.offset;
                visitNeighbour(nextPt, currPt,
                               offset.from, pt.distance, pt.level,
                               pt.group, currGroup, currLevelData,
                               prevGroupId, prevGroup,
                               offset.statsOnly);
            }

            *groupPtr = pt.group;

            if (progressUpdater && !(numFilledPixels & progressReportingMask)) {
                const int progressPercent =
                    qBound(0, qRound(100.0 * numFilledPixels / totalPixelsToFill), 100);
                progressUpdater->setProgress(progressPercent);
            }

        } else {
            // nothing to do?
        }

    }

    // cleaup iterators
    groupIt.clear();
    levelIt.clear();
    backgroundGroupId = 0;
    backgroundGroupColor = -1;
    recolorMode = false;

//    ENTER_FUNCTION() << ppVar(tt.elapsed());
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


    while (srcIt.nextPixel() && dstIt.nextPixel()) {
        const qint32 *srcPtr = reinterpret_cast<const qint32*>(srcIt.rawDataConst());

        const int colorIndex = groups[*srcPtr].colorIndex;
        if (colorIndex >= 0) {
            memcpy(dstIt.rawData(), colors[colorIndex].data(), colorPixelSize);
        }

    }
}

QVector<TaskPoint> KisWatershedWorker::Private::tryRemoveConflictingPlane(qint32 group, quint8 level)
{
    QVector<TaskPoint> result;

    FillGroup &g = groups[group];
    FillGroup::LevelData &l = g.levels[level];

    for (auto conflictIt = l.conflictWithGroup.begin(); conflictIt != l.conflictWithGroup.end(); ++conflictIt) {

        std::vector<QPoint> uniquePoints;
        std::unique_copy(conflictIt->begin(), conflictIt->end(), std::back_inserter(uniquePoints));

        for (auto pointIt = uniquePoints.begin(); pointIt != uniquePoints.end(); ++pointIt) {
            TaskPoint pt;
            pt.x = pointIt->x();
            pt.y = pointIt->y();
            pt.group = conflictIt.key();
            pt.level = level;

            result.append(pt);
            // no writing to the group map!
        }
    }

    return result;
}

void KisWatershedWorker::Private::updateNarrowRegionMetrics()
{
    for (qint32 i = 0; i < groups.size(); i++) {
        FillGroup &group = groups[i];

        for (auto levelIt = group.levels.begin(); levelIt != group.levels.end(); ++levelIt) {
            FillGroup::LevelData &l = levelIt.value();

            const qreal areaToPerimeterRatio = qreal(l.numFilledPixels) / l.totalEdgeSize();
            l.narrowRegion = areaToPerimeterRatio < 2.0;
        }
    }
}

QVector<GroupLevelPair> KisWatershedWorker::Private::calculateConflictingPairs()
{
    QVector<GroupLevelPair> result;


    for (qint32 i = 0; i < groups.size(); i++) {
        FillGroup &group = groups[i];

        for (auto levelIt = group.levels.begin(); levelIt != group.levels.end(); ++levelIt) {
            FillGroup::LevelData &l = levelIt.value();

            for (auto conflictIt = l.conflictWithGroup.begin(); conflictIt != l.conflictWithGroup.end(); ++conflictIt) {
                if (!conflictIt->empty()) {
                    result.append(GroupLevelPair(i, levelIt.key()));
                    break;
                }
            }
        }
    }

    return result;
}

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>

void KisWatershedWorker::Private::cleanupForeignEdgeGroups(qreal cleanUpAmount)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(cleanUpAmount > 0.0);

    // convert into the threshold range [0.05...0.5]
    const qreal foreignEdgePortionThreshold = 0.05 + 0.45 * (1.0 - qBound(0.0, cleanUpAmount, 1.0));

    QVector<GroupLevelPair> conflicts = calculateConflictingPairs();

    // sort the pairs by the total edge size
    QMap<qreal, GroupLevelPair> sortedPairs;
    Q_FOREACH (const GroupLevelPair &pair, conflicts) {
        const qint32 groupIndex = pair.first;
        const quint8 levelIndex = pair.second;
        FillGroup::LevelData &level = groups[groupIndex].levels[levelIndex];

        sortedPairs.insert(level.totalEdgeSize(), pair);
    }

    // remove sequentially from the smallest to the biggest
    for (auto pairIt = sortedPairs.begin(); pairIt != sortedPairs.end(); ++pairIt) {
        const qint32 groupIndex = pairIt->first;
        const quint8 levelIndex = pairIt->second;
        FillGroup::LevelData &level = groups[groupIndex].levels[levelIndex];

        const int thisLength = pairIt.key();
        const qreal thisForeignPortion = qreal(level.foreignEdgeSize) / thisLength;

        using namespace boost::accumulators;
        accumulator_set<int, stats<tag::count, tag::mean, tag::min>> lengthStats;

        for (auto it = level.conflictWithGroup.begin(); it != level.conflictWithGroup.end(); ++it) {
            const FillGroup::LevelData &otherLevel = groups[it.key()].levels[levelIndex];
            lengthStats(otherLevel.totalEdgeSize());
        }

        KIS_SAFE_ASSERT_RECOVER_BREAK(count(lengthStats));

        const qreal minMetric = min(lengthStats) / qreal(thisLength);
        const qreal meanMetric = mean(lengthStats) / qreal(thisLength);

//        qDebug() << "G" << groupIndex
//                 << "L" << levelIndex
//                 << "con" << level.conflictWithGroup.size()
//                 << "FRP" << thisForeignPortion
//                 << "S" << level.numFilledPixels
//                 << ppVar(thisLength)
//                 << ppVar(min(lengthStats))
//                 << ppVar(mean(lengthStats))
//                 << ppVar(minMetric)
//                 << ppVar(meanMetric);

        if (!(thisForeignPortion > foreignEdgePortionThreshold)) continue;

        if (minMetric > 1.0 && meanMetric > 1.2) {
//            qDebug() << "   * removing...";

            QVector<TaskPoint> taskPoints =
                tryRemoveConflictingPlane(groupIndex, levelIndex);

            if (!taskPoints.isEmpty()) {
                // dump before
                // dumpGroupInfo(groupIndex, levelIndex);

                Q_FOREACH (const TaskPoint &pt, taskPoints) {
                    pointsQueue.push(pt);
                }
                processQueue(groupIndex);

                // dump after: should become empty!
                // dumpGroupInfo(groupIndex, levelIndex);

                // the areas might be disjoint, so that removing one "conflicting"
                // part will not remove the whole group+level pair

                // KIS_SAFE_ASSERT_RECOVER_NOOP(level.totalEdgeSize() == 0);
            }

            //dumpGroupMaps();
            //calcNumGroupMaps();

        }
    }

}

void KisWatershedWorker::Private::dumpGroupMaps()
{
    KisPaintDeviceSP groupDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP colorDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP pedgeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP nedgeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP fedgeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    KisSequentialConstIterator heightIt(heightMap, boundingRect);
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



    while (dstGroupIt.nextPixel() &&
           heightIt.nextPixel() &&
           srcIt.nextPixel() &&
           dstColorIt.nextPixel() &&
           dstPedgeIt.nextPixel() &&
           dstNedgeIt.nextPixel() &&
           dstFedgeIt.nextPixel()) {

        const qint32 *srcPtr = reinterpret_cast<const qint32*>(srcIt.rawDataConst());

        *dstGroupIt.rawData() = quint8(*srcPtr);
        memcpy(dstColorIt.rawData(), colors[groups[*srcPtr].colorIndex].data(), colorPixelSize);

        quint8 level = *heightIt.rawDataConst();

        if (groups[*srcPtr].levels.contains(level)) {
            const FillGroup::LevelData &l = groups[*srcPtr].levels[level];

            const int edgeLength = l.totalEdgeSize();

            *dstPedgeIt.rawData() = 255.0 * qreal(l.positiveEdgeSize) / (edgeLength);
            *dstNedgeIt.rawData() = 255.0 * qreal(l.negativeEdgeSize) / (edgeLength);
            *dstFedgeIt.rawData() = 255.0 * qreal(l.foreignEdgeSize) / (edgeLength);
        } else {
            *dstPedgeIt.rawData() = 0;
            *dstNedgeIt.rawData() = 0;
            *dstFedgeIt.rawData() = 0;
        }
    }


    KIS_DUMP_DEVICE_2(groupDevice, boundingRect, "01_groupMap", "dd");
    KIS_DUMP_DEVICE_2(colorDevice, boundingRect, "02_colorMap", "dd");
    KIS_DUMP_DEVICE_2(pedgeDevice, boundingRect, "03_pedgeMap", "dd");
    KIS_DUMP_DEVICE_2(nedgeDevice, boundingRect, "04_nedgeMap", "dd");
    KIS_DUMP_DEVICE_2(fedgeDevice, boundingRect, "05_fedgeMap", "dd");
}

void KisWatershedWorker::Private::calcNumGroupMaps()
{
    KisSequentialConstIterator groupIt(groupsMap, boundingRect);
    KisSequentialConstIterator levelIt(heightMap, boundingRect);

    QSet<QPair<qint32, quint8>> groups;

    while (groupIt.nextPixel() && levelIt.nextPixel()) {

        const qint32 group = *reinterpret_cast<const qint32*>(groupIt.rawDataConst());
        const quint8 level = *reinterpret_cast<const quint8*>(levelIt.rawDataConst());

        groups.insert(qMakePair(group, level));
    }

    for (auto it = groups.begin(); it != groups.end(); ++it) {
        dumpGroupInfo(it->first, it->second);
    }

    ENTER_FUNCTION() << ppVar(groups.size());
}

void KisWatershedWorker::Private::dumpGroupInfo(qint32 groupIndex, quint8 levelIndex)
{
    FillGroup::LevelData &level = groups[groupIndex].levels[levelIndex];

    qDebug() << "G" << groupIndex << "L" << levelIndex << "CI" << this->groups[groupIndex].colorIndex;
    qDebug() << "   P" << level.positiveEdgeSize;
    qDebug() << "   N" << level.negativeEdgeSize;
    qDebug() << "   F" << level.foreignEdgeSize;
    qDebug() << "   A" << level.allyEdgeSize;
    qDebug() << " (S)" << level.numFilledPixels;

    auto &c = level.conflictWithGroup;

    for (auto cIt = c.begin(); cIt != c.end(); ++cIt) {
        qDebug() << "   C" << cIt.key() << cIt.value().size();
    }
}
