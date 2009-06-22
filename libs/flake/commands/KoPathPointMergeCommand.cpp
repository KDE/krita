/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathPointMergeCommand.h"
#include <KLocale>

/**
 * How does is work:
 *
 * The goal is to merge the point that is ending an open subpath with the one
 * starting the same or another open subpath.
 */
KoPathPointMergeCommand::KoPathPointMergeCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_pathShape(pointData1.pathShape)
        , m_endPoint(pointData1.pointIndex)
        , m_startPoint(pointData2.pointIndex)
        , m_splitIndex(KoPathPointIndex(-1, -1))
        , m_removedPoint(0)
        , m_reverse(ReverseNone)
{
    Q_ASSERT(pointData1.pathShape == pointData2.pathShape);
    Q_ASSERT(m_pathShape);
    Q_ASSERT(!m_pathShape->isClosedSubpath(m_endPoint.first));
    Q_ASSERT(m_endPoint.second == 0 ||
             m_endPoint.second == m_pathShape->pointCountSubpath(m_endPoint.first) - 1);
    Q_ASSERT(!m_pathShape->isClosedSubpath(m_startPoint.first));
    Q_ASSERT(m_startPoint.second == 0 ||
             m_startPoint.second == m_pathShape->pointCountSubpath(m_startPoint.first) - 1);

    // if we have two different subpaths we might need to reverse them
    if (m_endPoint.first != m_startPoint.first) {
        // sort by point index
        if (m_startPoint < m_endPoint)
            qSwap(m_endPoint, m_startPoint);
        // mark first subpath to be reversed if first point starts a subpath with more than one point 
        if (m_endPoint.second == 0 && m_pathShape->pointCountSubpath(m_endPoint.first) > 1)
            m_reverse |= ReverseFirst;
        // mark second subpath to be reversed if second point does not start a subpath with more than one point
        if (m_startPoint.second != 0 && m_pathShape->pointCountSubpath(m_startPoint.first) > 1)
            m_reverse |= ReverseSecond;
    } else {
        Q_ASSERT(m_endPoint.second != m_startPoint.second);
        if (m_endPoint < m_startPoint)
            qSwap(m_endPoint, m_startPoint);
    }
    
    KoPathPoint * p1 = m_pathShape->pointByIndex(m_endPoint);
    KoPathPoint * p2 = m_pathShape->pointByIndex(m_startPoint);
    
    m_oldNodePoint1 = m_pathShape->shapeToDocument(p1->point());
    if (m_reverse & ReverseFirst) {
        m_oldControlPoint1 = m_pathShape->shapeToDocument(p1->controlPoint2());
    } else {
        m_oldControlPoint1 = m_pathShape->shapeToDocument(p1->controlPoint1());
    }
    m_oldNodePoint2 = m_pathShape->shapeToDocument(p2->point());
    if (m_reverse & ReverseSecond) {
        m_oldControlPoint2 = m_pathShape->shapeToDocument(p2->controlPoint1());
    } else {
        m_oldControlPoint2 = m_pathShape->shapeToDocument(p2->controlPoint2());
    }
    
    setText(i18n("Merge points"));
}

KoPathPointMergeCommand::~KoPathPointMergeCommand()
{
    delete m_removedPoint;
}

KoPathPoint * KoPathPointMergeCommand::mergePoints( KoPathPoint * p1, KoPathPoint * p2)
{
    QPointF mergePosition = 0.5 * (p1->point() + p2->point());
    QPointF mergeControlPoint1 = mergePosition + (p1->controlPoint1() - p1->point());
    QPointF mergeControlPoint2 = mergePosition + (p2->controlPoint2() - p2->point());

    // change position and control points of first merged point
    p1->setPoint( mergePosition );
    if (p1->activeControlPoint1()) {
        p1->setControlPoint1(mergeControlPoint1);
    }
    if (p2->activeControlPoint2()) {
        p1->setControlPoint2(mergeControlPoint2);
    }
    
    // remove the second merged point
    KoPathPointIndex removeIndex = m_pathShape->pathPointIndex(p2);
    return m_pathShape->removePoint(removeIndex);
}

void KoPathPointMergeCommand::redo()
{
    QUndoCommand::redo();
    
    if (m_removedPoint)
        return;
    
    m_pathShape->update();
    
    KoPathPoint * endPoint = m_pathShape->pointByIndex(m_endPoint);
    KoPathPoint * startPoint = m_pathShape->pointByIndex(m_startPoint);

    // are we just closing a single subpath ?
    if (m_endPoint.first == m_startPoint.first) {
        // change the endpoint of the subpath
        m_removedPoint = mergePoints(endPoint, startPoint);
        // set endpoint of subpath to close the subpath
        endPoint->setProperty(KoPathPoint::CloseSubpath);
        // set new startpoint of subpath to close the subpath
        KoPathPointIndex newStartIndex(m_startPoint.first,0); 
        m_pathShape->pointByIndex(newStartIndex)->setProperty(KoPathPoint::CloseSubpath);
    } else {
        // first revert subpathes if needed
        if (m_reverse & ReverseFirst) {
            m_pathShape->reverseSubpath(m_endPoint.first);
        }
        if (m_reverse & ReverseSecond) {
            m_pathShape->reverseSubpath(m_startPoint.first);
        }
        // move the subpaths so the second is directly after the first
        m_pathShape->moveSubpath(m_startPoint.first, m_endPoint.first + 1);
        m_splitIndex = m_pathShape->pathPointIndex(endPoint);
        // join both subpathes
        m_pathShape->join(m_endPoint.first);
        // change the first point of the points to merge
        m_removedPoint = mergePoints(endPoint, startPoint);
    }
    
    m_pathShape->normalize();
    m_pathShape->update();
}

void KoPathPointMergeCommand::resetPoints( KoPathPointIndex index1, KoPathPointIndex index2 )
{
    KoPathPoint * p1 = m_pathShape->pointByIndex(index1);
    KoPathPoint * p2 = m_pathShape->pointByIndex(index2);
    p1->setPoint(m_pathShape->documentToShape(m_oldNodePoint1));
    p2->setPoint(m_pathShape->documentToShape(m_oldNodePoint2));
    if (p1->activeControlPoint1()) {
        p1->setControlPoint1(m_pathShape->documentToShape(m_oldControlPoint1));
    }
    if (p2->activeControlPoint2()) {
        p2->setControlPoint2(m_pathShape->documentToShape(m_oldControlPoint2));
    }
}

void KoPathPointMergeCommand::undo()
{
    QUndoCommand::undo();
    
    if (!m_removedPoint)
        return;
    
    m_pathShape->update();

    // check if we just have closed a single subpath
    if (m_endPoint.first == m_startPoint.first) {
        // open the subpath at the old/new first point
        m_pathShape->openSubpath(m_startPoint);
        // reinsert the old first point
        m_pathShape->insertPoint(m_removedPoint, m_startPoint);
        // reposition the points
        resetPoints(m_endPoint, m_startPoint);
    } else {
        // break merged subpathes apart
        m_pathShape->breakAfter(m_splitIndex);
        // reinsert the old second point
        m_pathShape->insertPoint(m_removedPoint, KoPathPointIndex(m_splitIndex.first+1,0));
        // reposition the first point
        resetPoints(m_splitIndex, KoPathPointIndex(m_splitIndex.first+1,0));
        // move second subpath to its old position
        m_pathShape->moveSubpath(m_splitIndex.first+1, m_startPoint.first);
        // undo the reversion of the subpaths
        if (m_reverse & ReverseFirst) {
            m_pathShape->reverseSubpath(m_endPoint.first);
        }
        if (m_reverse & ReverseSecond) {
            m_pathShape->reverseSubpath(m_startPoint.first);
        }
    }

    m_pathShape->normalize();
    m_pathShape->update();
    
    // reset the removed point
    m_removedPoint = 0;
}
