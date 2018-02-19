/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPathToolSelection.h"
#include "KoPathTool.h"
#include <KoParameterShape.h>
#include <KoPathPoint.h>
#include <KoPathPointData.h>
#include <KoViewConverter.h>
#include <KoCanvasBase.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>
#include <QPainter>
#include <KoShapeHandlesCollection.h>
#include <KoCanvasUpdatesCollector.h>


KoPathToolSelection::KoPathToolSelection(KoPathTool * tool)
        : m_tool(tool)
{
}

KoPathToolSelection::~KoPathToolSelection()
{
}

void KoPathToolSelection::add(KoPathPoint *point, bool clear, KoCanvasUpdatesCollector &pendingUpdates)
{
    if(!point)
        return;

    bool allreadyIn = false;
    if (clear) {
        if (size() == 1 && m_selectedPoints.contains(point)) {
            allreadyIn = true;
        } else {
            this->clear(pendingUpdates);
        }
    } else {
        allreadyIn = m_selectedPoints.contains(point);
    }

    if (!allreadyIn) {
        m_selectedPoints.insert(point);
        KoPathShape * pathShape = point->parent();
        PathShapePointMap::iterator it(m_shapePointMap.find(pathShape));
        if (it == m_shapePointMap.end()) {
            it = m_shapePointMap.insert(pathShape, QSet<KoPathPoint *>());
        }
        it.value().insert(point);

        pendingUpdates.addUpdate(
            KoShapeHandlesCollection::updateDocRects(
                KoFlake::HandlesRecord(point->parent(),
                                                        KisHandleStyle::selectedPrimaryHandles(),
                                                        point->handles(KoPathPoint::All)),
                m_tool->handleRadius()));
        emit selectionChanged();
    }
}

void KoPathToolSelection::remove(KoPathPoint * point, KoCanvasUpdatesCollector &pendingUpdates)
{
    pendingUpdates.addUpdate(
        KoShapeHandlesCollection::updateDocRects(
            KoFlake::HandlesRecord(point->parent(),
                                                    KisHandleStyle::selectedPrimaryHandles(),
                                                    point->handles(KoPathPoint::All)),
            m_tool->handleRadius()));

    if (m_selectedPoints.remove(point)) {
        KoPathShape * pathShape = point->parent();
        m_shapePointMap[pathShape].remove(point);
        if (m_shapePointMap[pathShape].size() == 0) {
            m_shapePointMap.remove(pathShape);
        }
        emit selectionChanged();
    }
}

void KoPathToolSelection::clear(KoCanvasUpdatesCollector &pendingUpdates)
{
    pendingUpdates.addUpdate(
        KoShapeHandlesCollection::updateDocRects(
            collectSelectedHandles(KisHandleStyle::selectedPrimaryHandles()),
                                 m_tool->handleRadius()));

    m_selectedPoints.clear();
    m_shapePointMap.clear();
    emit selectionChanged();
}

void KoPathToolSelection::selectPoints(const QRectF &rect, bool clearSelection, KoCanvasUpdatesCollector &pendingUpdates)
{
    if (clearSelection) {
        clear(pendingUpdates);
    }

    blockSignals(true);
    Q_FOREACH (KoPathShape* shape, m_selectedShapes) {
        KoParameterShape *parameterShape = dynamic_cast<KoParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape())
            continue;
        Q_FOREACH (KoPathPoint* point, shape->pointsAt(shape->documentToShape(rect)))
            add(point, false, pendingUpdates);
    }
    blockSignals(false);
    emit selectionChanged();
}

int KoPathToolSelection::objectCount() const
{
    return m_shapePointMap.size();
}

int KoPathToolSelection::size() const
{
    return m_selectedPoints.size();
}

bool KoPathToolSelection::contains(KoPathPoint * point)
{
    return m_selectedPoints.contains(point);
}

const QSet<KoPathPoint *> & KoPathToolSelection::selectedPoints() const
{
    return m_selectedPoints;
}

QList<KoPathPointData> KoPathToolSelection::selectedPointsData() const
{
    QList<KoPathPointData> pointData;
    Q_FOREACH (KoPathPoint* p, m_selectedPoints) {
        KoPathShape * pathShape = p->parent();
        pointData.append(KoPathPointData(pathShape, pathShape->pathPointIndex(p)));
    }
    return pointData;
}

QList<KoPathPointData> KoPathToolSelection::selectedSegmentsData() const
{
    QList<KoPathPointData> pointData;

    QList<KoPathPointData> pd(selectedPointsData());
    std::sort(pd.begin(), pd.end());

    KoPathPointData last(0, KoPathPointIndex(-1, -1));
    KoPathPointData lastSubpathStart(0, KoPathPointIndex(-1, -1));

    QList<KoPathPointData>::const_iterator it(pd.constBegin());
    for (; it != pd.constEnd(); ++it) {
        if (it->pointIndex.second == 0)
            lastSubpathStart = *it;

        if (last.pathShape == it->pathShape
                && last.pointIndex.first == it->pointIndex.first
                && last.pointIndex.second + 1 == it->pointIndex.second) {
            pointData.append(last);
        }

        if (lastSubpathStart.pathShape == it->pathShape
                && it->pathShape->pointByIndex(it->pointIndex)->properties() & KoPathPoint::CloseSubpath
                && (it->pathShape->pointByIndex(it->pointIndex)->properties() & KoPathPoint::StartSubpath) == 0) {
            pointData.append(*it);
        }

        last = *it;
    }

    return pointData;
}

QList<KoPathShape*> KoPathToolSelection::selectedShapes() const
{
    return m_selectedShapes;
}

void KoPathToolSelection::setSelectedShapes(const QList<KoPathShape*> shapes)
{
    m_selectedShapes = shapes;
}

QVector<KoFlake::HandlesRecord>
KoPathToolSelection::collectSelectedHandles(const KisHandleStyle &style)
{
    QVector<KoFlake::HandlesRecord> records;

    PathShapePointMap::iterator it(m_shapePointMap.begin());
    for (; it != m_shapePointMap.end(); ++it) {
        Q_FOREACH (KoPathPoint *p, it.value()) {
            records.append(KoFlake::HandlesRecord(p->parent(), style, p->handles(KoPathPoint::All)));
        }
    }

    return records;
}

#include <KoShapeStrokeModel.h>

QVector<KoFlake::HandlesRecord> KoPathToolSelection::collectShapeHandles()
{
    QVector<KoFlake::HandlesRecord> records;

    Q_FOREACH (KoPathShape *shape, selectedShapes()) {
        if (!shape->stroke() || !shape->stroke()->isVisible()) {
            records.append(KoFlake::HandlesRecord(
                shape,
                KisHandleStyle::secondarySelection(),
                {KritaUtils::Handle(KritaUtils::OutlinePath, shape->outline())}));
        }

        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape()) {
            for (int i = 0; i < parameterShape->handleCount(); i++) {
                records.append(KoFlake::HandlesRecord(
                    shape,
                    KisHandleStyle::primarySelection(),
                    {parameterShape->handleObject(i)}));
            }
        } else {
            QVector<KoPathPoint*> allPoints = shape->allPathPoints();
            Q_FOREACH (KoPathPoint *pt, allPoints) {
                records.append(KoFlake::HandlesRecord(
                   shape, KisHandleStyle::primarySelection(),
                   pt->handles(KoPathPoint::Node)));
            }
        }
    }

    return records;
}

void KoPathToolSelection::update()
{
    bool selectionHasChanged = false;

    PathShapePointMap::iterator it(m_shapePointMap.begin());
    while (it != m_shapePointMap.end()) {
        KoParameterShape *parameterShape = dynamic_cast<KoParameterShape*>(it.key());
        bool isParametricShape = parameterShape && parameterShape->isParametricShape();
        if (! m_selectedShapes.contains(it.key()) || isParametricShape) {
            QSet<KoPathPoint *>::iterator pointIt(it.value().begin());
            for (; pointIt != it.value().end(); ++pointIt) {
                m_selectedPoints.remove(*pointIt);
            }
            it = m_shapePointMap.erase(it);
            selectionHasChanged = true;
        } else {
            QSet<KoPathPoint *>::iterator pointIt(it.value().begin());
            while (pointIt != it.value().end()) {
                if ((*pointIt)->parent()->pathPointIndex(*pointIt) == KoPathPointIndex(-1, -1)) {
                    m_selectedPoints.remove(*pointIt);
                    pointIt = it.value().erase(pointIt);
                    selectionHasChanged = true;
                } else {
                    ++pointIt;
                }
            }
            ++it;
        }
    }

    if (selectionHasChanged)
        emit selectionChanged();
}

bool KoPathToolSelection::hasSelection()
{
    return !m_selectedPoints.isEmpty();
}
