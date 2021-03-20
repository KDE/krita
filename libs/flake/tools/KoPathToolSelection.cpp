/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <KisHandlePainterHelper.h>

KoPathToolSelection::KoPathToolSelection(KoPathTool * tool)
        : m_tool(tool)
{
}

KoPathToolSelection::~KoPathToolSelection()
{
}

void KoPathToolSelection::paint(QPainter &painter, const KoViewConverter &converter, qreal handleRadius)
{
    PathShapePointMap::iterator it(m_shapePointMap.begin());
    for (; it != m_shapePointMap.end(); ++it) {
        KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelperView(&painter, it.key(), converter, handleRadius);
        helper.setHandleStyle(KisHandleStyle::selectedPrimaryHandles());

        Q_FOREACH (KoPathPoint *p, it.value()) {
            p->paint(helper, KoPathPoint::All);
        }
    }
}

void KoPathToolSelection::add(KoPathPoint * point, bool clear)
{
    if(! point)
        return;

    bool allreadyIn = false;
    if (clear) {
        if (size() == 1 && m_selectedPoints.contains(point)) {
            allreadyIn = true;
        } else {
            this->clear();
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
        emit selectionChanged();
    }
}

void KoPathToolSelection::remove(KoPathPoint * point)
{
    if (m_selectedPoints.remove(point)) {
        KoPathShape * pathShape = point->parent();
        m_shapePointMap[pathShape].remove(point);
        if (m_shapePointMap[pathShape].size() == 0) {
            m_shapePointMap.remove(pathShape);
        }
        emit selectionChanged();
    }
}

void KoPathToolSelection::clear()
{
    m_selectedPoints.clear();
    m_shapePointMap.clear();
    emit selectionChanged();
}

void KoPathToolSelection::selectPoints(const QRectF &rect, bool clearSelection)
{
    if (clearSelection) {
        clear();
    }

    blockSignals(true);
    Q_FOREACH (KoPathShape* shape, m_selectedShapes) {
        KoParameterShape *parameterShape = dynamic_cast<KoParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape())
            continue;
        Q_FOREACH (KoPathPoint* point, shape->pointsAt(shape->documentToShape(rect)))
            add(point, false);
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
    Q_FOREACH(KoPathShape *shape, m_selectedShapes) {
        shape->removeShapeChangeListener(this);
    }

    m_selectedShapes = shapes;

    Q_FOREACH(KoPathShape *shape, m_selectedShapes) {
        shape->addShapeChangeListener(this);
    }
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

void KoPathToolSelection::recommendPointSelectionChange(KoPathShape *shape, const QList<KoPathPointIndex> &newSelection)
{
    QSet<KoPathPoint*> selectedShapePoints = m_shapePointMap.value(shape, QSet<KoPathPoint*>());

    Q_FOREACH (KoPathPoint *point, selectedShapePoints) {
        remove(point);
    }

    Q_FOREACH (const KoPathPointIndex &index, newSelection) {
        KoPathPoint *point = shape->pointByIndex(index);
        KIS_SAFE_ASSERT_RECOVER(point) { continue; }

        add(point, false);
    }

    emit selectionChanged();
}

void KoPathToolSelection::notifyPathPointsChanged(KoPathShape *shape)
{
    QSet<KoPathPoint*> selectedShapePoints = m_shapePointMap.value(shape, QSet<KoPathPoint*>());

    Q_FOREACH (KoPathPoint *point, selectedShapePoints) {
        m_selectedPoints.remove(point);
    }
    m_shapePointMap.remove(shape);

    m_tool->notifyPathPointsChanged(shape);

    emit selectionChanged();
}

void KoPathToolSelection::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    if (type == KoShape::Deleted) {
        // we cannot select non-path shapes, so static cast is safe here
        KIS_SAFE_ASSERT_RECOVER_NOOP(shape->shapeId() == KoPathShapeId);

        if (KoPathShape *pathShape = static_cast<KoPathShape*>(shape)) {

            QSet<KoPathPoint*> selectedShapePoints = m_shapePointMap.value(pathShape, QSet<KoPathPoint*>());
            Q_FOREACH (KoPathPoint *point, selectedShapePoints) {
                m_selectedPoints.remove(point);
            }
            m_shapePointMap.remove(pathShape);
            m_selectedShapes.removeAll(pathShape);
        }
    }

    KoPathShape::PointSelectionChangeListener::notifyShapeChanged(type, shape);
}
