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

#ifndef KOPATHPOINTTYPECOMMAND_H
#define KOPATHPOINTTYPECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "KoPathBaseCommand.h"
//#include "KoPathShape.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "flake_export.h"

/// The undo / redo command for changing the path point type.
class FLAKE_TEST_EXPORT KoPathPointTypeCommand : public KoPathBaseCommand
{
public:
    /// The type of the point
    enum PointType {
        Corner,
        Smooth,
        Symmetric,
        Line,
        Curve
    };
    /**
     * Command to change the type of the given points
     * @param pointDataList List of point for changing the points
     * @param pointType the new point type to set
     * @param parent the parent command used for macro commands
     */
    KoPathPointTypeCommand(const QList<KoPathPointData> &pointDataList, PointType pointType, QUndoCommand *parent = 0);
    ~KoPathPointTypeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    // used for storing the data for undo
    struct PointData {
        PointData(const KoPathPointData pointData)
                : m_pointData(pointData) {}
        KoPathPointData m_pointData;
        // old control points in document coordinates
        QPointF m_oldControlPoint1;
        QPointF m_oldControlPoint2;
        KoPathPoint::PointProperties m_oldProperties;
        bool m_hadControlPoint1;
        bool m_hadControlPoint2;
    };

    bool appendPointData(KoPathPointData data);
    void undoChanges(const QList<PointData> &data);

    PointType m_pointType;
    QList<PointData> m_oldPointData;
    QList<PointData> m_additionalPointData;
};

#endif // KOPATHPOINTTYPECOMMAND_H
