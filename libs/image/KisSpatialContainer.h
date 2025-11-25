/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SPATIAL_CONTAINER_H
#define __KIS_SPATIAL_CONTAINER_H

#include <QPoint>
#include <QPointF>
#include <QVector>
#include <QPolygonF>
#include <QTransform>
#include <QDebug>
#include <cmath>
#include <kis_global.h>
#include <kritaimage_export.h>
#include <boost/optional.hpp>
#include <optional>


class KRITAIMAGE_EXPORT KisSpatialContainer
{
    // right now it's just a standard Quadtree, since it seemed like it would work well
    // for points that are mostly uniformly located
    // and need a decent amount of changes and need to have fast queries


public:
    struct SpatialNode;

public:


    KisSpatialContainer(QRectF startArea, int maxPointsInDict = 100);
    KisSpatialContainer(QRectF startArea, QVector<QPointF> &points);
    KisSpatialContainer(const KisSpatialContainer &rhs);

    ~KisSpatialContainer();

    void initializeFor(int numPoints, QRectF startArea);
    void initializeWith(const QVector<QPointF> &points);

    void initializeWithGridPoints(QRectF gridRect, int pixelPrecision);

    void addPoint(int index, QPointF position);
    void removePoint(int index, QPointF position);
    void movePoint(int index, QPointF positionBefore, QPointF positionAfter);
    QVector<QPointF> toVector();

    void findAllInRange(QVector<int> &indexes, QPointF center, qreal range);

    int count();
    // O(log(n)*m_maxPointsInDict)
    QPointF getTopLeft();
    // O(log(n)*m_maxPointsInDict)
    QRectF exactBounds();

    // erase everything
    void clear();


    void debugWriteOut();


private:

    void addPointRec(int index, QPointF position, SpatialNode* node);
    void removePointRec(int index, QPointF position, SpatialNode* node);
    void movePointRec(int index, QPointF positionBefore, QPointF positionAfter, SpatialNode* node);
    SpatialNode *createNodeForPoint(int index, QPointF position);


    void findAllInRangeRec(QVector<int> &indexes, QPointF center, qreal range, SpatialNode* node);


    void gatherDataRec(QVector<QPointF> &vector, SpatialNode* node);

    void debugWriteOutRec(SpatialNode* node, QString prefix);

    std::optional<qreal> getBoundaryOnAxis(bool positive, bool xAxis, SpatialNode* node);
    QPointF getBoundaryPoint(bool left, bool top);

    void initializeLevels(SpatialNode* node, int levelsLeft, QRectF area);
    void initializeWithGridPointsRec(QRectF gridRect, int pixelPrecision, SpatialNode* node, int startRow, int startColumn, int columnCount);

    void clearRec(SpatialNode* node);

    void deepCopyData(SpatialNode* node, const SpatialNode* from);



private:

    int m_count {0};
    int m_maxPointsInDict = {100};
    int m_nextNodeId = {0}; // just for debug
    SpatialNode* m_root {nullptr};

    friend class KisSpatialContainerTest;


};





#endif /* __KIS_SPATIAL_CONTAINER_H */
