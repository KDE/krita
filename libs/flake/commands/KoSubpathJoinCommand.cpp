/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoSubpathJoinCommand.h"
#include <klocalizedstring.h>
#include "kis_assert.h"
#include "KoPathMergeUtils.h"


KoSubpathJoinCommand::KoSubpathJoinCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, KUndo2Command *parent)
        : KUndo2Command(parent)
        , m_pointData1(pointData1)
        , m_pointData2(pointData2)
        , m_splitIndex(KoPathPointIndex(-1, -1))
        , m_oldProperties1(KoPathPoint::Normal)
        , m_oldProperties2(KoPathPoint::Normal)
        , m_reverse(0)
{
    KIS_ASSERT(m_pointData1.pathShape == m_pointData2.pathShape);
    KoPathShape * pathShape = m_pointData1.pathShape;
    KIS_ASSERT(!pathShape->isClosedSubpath(m_pointData1.pointIndex.first));
    KIS_ASSERT(m_pointData1.pointIndex.second == 0 ||
               m_pointData1.pointIndex.second == pathShape->subpathPointCount(m_pointData1.pointIndex.first) - 1);
    KIS_ASSERT(!pathShape->isClosedSubpath(m_pointData2.pointIndex.first));
    KIS_ASSERT(m_pointData2.pointIndex.second == 0 ||
               m_pointData2.pointIndex.second == pathShape->subpathPointCount(m_pointData2.pointIndex.first) - 1);
    //TODO check that points are not the same

    if (m_pointData2 < m_pointData1) {
        std::swap(m_pointData1, m_pointData2);
    }

    if (!closeSubpathMode()) {
        if (m_pointData1.pointIndex.second == 0 &&
            pathShape->subpathPointCount(m_pointData1.pointIndex.first) > 1)
            m_reverse |= ReverseFirst;
        if (m_pointData2.pointIndex.second != 0)
            m_reverse |= ReverseSecond;
        setText(kundo2_i18n("Join subpaths"));
    } else {
        setText(kundo2_i18n("Close subpath"));
    }

    KoPathPoint * point1 = pathShape->pointByIndex(m_pointData1.pointIndex);
    KoPathPoint * point2 = pathShape->pointByIndex(m_pointData2.pointIndex);

    m_savedControlPoint1 = KritaUtils::fetchControlPoint(point1, m_reverse & ReverseFirst);
    m_savedControlPoint2 = KritaUtils::fetchControlPoint(point2, !(m_reverse & ReverseSecond));

    m_oldProperties1 = point1->properties();
    m_oldProperties2 = point2->properties();
}

KoSubpathJoinCommand::~KoSubpathJoinCommand()
{
}



void KoSubpathJoinCommand::redo()
{
    KUndo2Command::redo();
    KoPathShape * pathShape = m_pointData1.pathShape;

    KoPathPoint * point1 = pathShape->pointByIndex(m_pointData1.pointIndex);
    KoPathPoint * point2 = pathShape->pointByIndex(m_pointData2.pointIndex);

    KIS_SAFE_ASSERT_RECOVER_RETURN(point1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(point2);

    // if the endpoint has a control point create a control point for the new segment to be
    // at the symmetric position to the exiting one

    if (closeSubpathMode()) {
        KritaUtils::makeSymmetric(point1, false);
        KritaUtils::makeSymmetric(point2, true);
    } else {
        KritaUtils::makeSymmetric(point1, !(m_reverse & ReverseFirst));
        KritaUtils::makeSymmetric(point2, m_reverse & ReverseSecond);
    }

    if (closeSubpathMode()) {
        pathShape->closeSubpath(m_pointData1.pointIndex);
    } else {
        if (m_reverse & ReverseFirst) {
            pathShape->reverseSubpath(m_pointData1.pointIndex.first);
        }
        if (m_reverse & ReverseSecond) {
            pathShape->reverseSubpath(m_pointData2.pointIndex.first);
        }
        pathShape->moveSubpath(m_pointData2.pointIndex.first, m_pointData1.pointIndex.first + 1);
        m_splitIndex = m_pointData1.pointIndex;
        m_splitIndex.second = pathShape->subpathPointCount(m_pointData1.pointIndex.first) - 1;
        pathShape->join(m_pointData1.pointIndex.first);
    }

    QList<KoPathPointIndex> pointIndexes;
    pointIndexes << pathShape->pathPointIndex(point1);
    pointIndexes << pathShape->pathPointIndex(point2);
    pathShape->recommendPointSelectionChange(pointIndexes);

    pathShape->normalize();
    pathShape->update();
}

void KoSubpathJoinCommand::undo()
{
    KUndo2Command::undo();
    KoPathShape * pathShape = m_pointData1.pathShape;
    pathShape->update();

    if (closeSubpathMode()) {
        pathShape->openSubpath(m_pointData1.pointIndex);
    } else {
        pathShape->breakAfter(m_splitIndex);
        pathShape->moveSubpath(m_pointData1.pointIndex.first + 1, m_pointData2.pointIndex.first);

        if (m_reverse & ReverseSecond) {
            pathShape->reverseSubpath(m_pointData2.pointIndex.first);
        }
        if (m_reverse & ReverseFirst) {
            pathShape->reverseSubpath(m_pointData1.pointIndex.first);
        }
    }
    KoPathPoint * point1 = pathShape->pointByIndex(m_pointData1.pointIndex);
    KoPathPoint * point2 = pathShape->pointByIndex(m_pointData2.pointIndex);

    KIS_SAFE_ASSERT_RECOVER_RETURN(point1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(point2);

    // restore the old end points
    if (closeSubpathMode()) {
        KritaUtils::restoreControlPoint(point1, true, m_savedControlPoint1);
        KritaUtils::restoreControlPoint(point2, false, m_savedControlPoint2);
    } else {
        KritaUtils::restoreControlPoint(point1, m_reverse & ReverseFirst, m_savedControlPoint1);
        KritaUtils::restoreControlPoint(point2, !(m_reverse & ReverseSecond), m_savedControlPoint2);
    }

    point1->setProperties(m_oldProperties1);
    point2->setProperties(m_oldProperties2);

    QList<KoPathPointIndex> pointIndexes;
    pointIndexes << pathShape->pathPointIndex(point1);
    pointIndexes << pathShape->pathPointIndex(point2);
    pathShape->recommendPointSelectionChange(pointIndexes);

    pathShape->normalize();
    pathShape->update();
}

bool KoSubpathJoinCommand::closeSubpathMode() const
{
    return m_pointData1.pointIndex.first == m_pointData2.pointIndex.first;
}

