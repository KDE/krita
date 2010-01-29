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

#ifndef KOSUBPATHJOINCOMMAND_H
#define KOSUBPATHJOINCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include "KoPathPoint.h"
#include "KoPathPointData.h"

/// The undo / redo command for joining two subpath end points
class KoSubpathJoinCommand : public QUndoCommand
{
public:
    /**
     * Command to join two subpath end points.
     *
     * The points have to be from the same path shape.
     *
     * @param pointData1 the data of the first point to join
     * @param pointData2 the data of the second point to join
     * @param parent the parent command used for macro commands
     */
    KoSubpathJoinCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, QUndoCommand *parent = 0);
    ~KoSubpathJoinCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathPointData m_pointData1;
    KoPathPointData m_pointData2;
    KoPathPointIndex m_splitIndex;
    // the control points have to be stored in document positions
    QPointF m_oldControlPoint1;
    QPointF m_oldControlPoint2;
    KoPathPoint::PointProperties m_oldProperties1;
    KoPathPoint::PointProperties m_oldProperties2;
    enum Reverse {
        ReverseFirst = 1,
        ReverseSecond = 2
    };
    int m_reverse;
};

#endif // KOSUBPATHJOINCOMMAND_H
