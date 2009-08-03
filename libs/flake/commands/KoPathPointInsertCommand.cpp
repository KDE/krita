/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathPointInsertCommand.h"
#include <klocale.h>

KoPathPointInsertCommand::KoPathPointInsertCommand(const QList<KoPathPointData> & pointDataList, qreal insertPosition, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_deletePoints(true)
{
    if (insertPosition < 0)
        insertPosition = 0;
    if (insertPosition > 1)
        insertPosition = 1;

    //TODO the list needs to be sorted

    QList<KoPathPointData>::const_iterator it(pointDataList.begin());
    for (; it != pointDataList.end(); ++it) {
        KoPathShape * pathShape = it->pathShape;

        KoPathSegment segment = pathShape->segmentByIndex(it->pointIndex);

        // should not happen but to be sure
        if (! segment.isValid())
            continue;

        m_pointDataList.append(*it);

        QPair<KoPathSegment, KoPathSegment> splitSegments = segment.splitAt( insertPosition );

        KoPathPoint * split1 = splitSegments.first.second();
        KoPathPoint * split2 = splitSegments.second.first();
        KoPathPoint * splitPoint = new KoPathPoint( pathShape, split1->point() );
        if( split1->activeControlPoint1() )
            splitPoint->setControlPoint1(split1->controlPoint1());
        if( split2->activeControlPoint2() )
            splitPoint->setControlPoint2(split2->controlPoint2());

        m_points.append(splitPoint);
        QPointF cp1 = splitSegments.first.first()->controlPoint2();
        QPointF cp2 = splitSegments.second.second()->controlPoint1();
        m_controlPoints.append(QPair<QPointF, QPointF>(cp1, cp2));
    }
}

KoPathPointInsertCommand::~KoPathPointInsertCommand()
{
    if (m_deletePoints) {
        qDeleteAll(m_points);
    }
}

void KoPathPointInsertCommand::redo()
{
    QUndoCommand::redo();
    for (int i = m_pointDataList.size() - 1; i >= 0; --i) {
        KoPathPointData pointData = m_pointDataList.at(i);
        KoPathShape * pathShape = pointData.pathShape;

        KoPathSegment segment = pathShape->segmentByIndex(pointData.pointIndex);

        ++pointData.pointIndex.second;

        if (segment.first()->activeControlPoint2()) {
            QPointF controlPoint2 = segment.first()->controlPoint2();
            qSwap(controlPoint2, m_controlPoints[i].first);
            segment.first()->setControlPoint2(controlPoint2);
        }

        if (segment.second()->activeControlPoint1()) {
            QPointF controlPoint1 = segment.second()->controlPoint1();
            qSwap(controlPoint1, m_controlPoints[i].second);
            segment.second()->setControlPoint1(controlPoint1);
        }

        pathShape->insertPoint(m_points.at(i), pointData.pointIndex);
        pathShape->update();
    }
    m_deletePoints = false;
}

void KoPathPointInsertCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < m_pointDataList.size(); ++i) {
        const KoPathPointData &pdBefore = m_pointDataList.at(i);
        KoPathShape * pathShape = pdBefore.pathShape;
        KoPathPointIndex piAfter = pdBefore.pointIndex;
        ++piAfter.second;

        KoPathPoint * before = pathShape->pointByIndex(pdBefore.pointIndex);

        m_points[i] = pathShape->removePoint(piAfter);

        if (m_points[i]->properties() & KoPathPoint::CloseSubpath) {
            piAfter.second = 0;
        }

        KoPathPoint * after = pathShape->pointByIndex(piAfter);

        if (before->activeControlPoint2()) {
            QPointF controlPoint2 = before->controlPoint2();
            qSwap(controlPoint2, m_controlPoints[i].first);
            before->setControlPoint2(controlPoint2);
        }

        if (after->activeControlPoint1()) {
            QPointF controlPoint1 = after->controlPoint1();
            qSwap(controlPoint1, m_controlPoints[i].second);
            after->setControlPoint1(controlPoint1);
        }
        pathShape->update();
    }
    m_deletePoints = true;
}

QList<KoPathPoint*> KoPathPointInsertCommand::insertedPoints() const
{
    return m_points;
}
