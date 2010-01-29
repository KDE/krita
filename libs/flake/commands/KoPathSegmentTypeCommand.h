/* This file is part of the KDE project
 * Copyright (C) 2006,2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHSEGMENTTYPECOMMAND_H
#define KOPATHSEGMENTTYPECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QPointF>
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "flake_export.h"

/// The undo / redo command for changing segments to curves/lines
class FLAKE_TEST_EXPORT KoPathSegmentTypeCommand : public QUndoCommand
{
public:
    /// Segment Types
    enum SegmentType {
        Curve = 1,
        Line = 2
    };

    /**
    * Command for changing the segment type ( curve/line )
    * @param pointData point data identifying the segement that should be changed.
    * @param segmentType to which the segment should be changed to
    * @param parent the parent command used for macro commands
    */
    KoPathSegmentTypeCommand(const KoPathPointData &pointData, SegmentType segmentType, QUndoCommand *parent = 0);

    /**
     * Command for changing the segment type ( curve/line )
     * @param pointDataList List of point data identifying the segements that should be changed.
     * @param segmentType to which the segments should be changed to
     * @param parent the parent command used for macro commands
     */
    KoPathSegmentTypeCommand(const QList<KoPathPointData> &pointDataList, SegmentType segmentType, QUndoCommand *parent = 0);
    ~KoPathSegmentTypeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    // used for storing the data for undo
    struct SegmentTypeData {
        // old control points in document coordinates
        QPointF m_controlPoint1;
        QPointF m_controlPoint2;
        KoPathPoint::PointProperties m_properties1;
        KoPathPoint::PointProperties m_properties2;
    };

    void initialize(const QList<KoPathPointData> &pointDataList);

    QList<KoPathPointData> m_pointDataList;
    QList<SegmentTypeData> m_segmentData;
    SegmentType m_segmentType;
};

#endif // KOPATHSEGMENTTYPECOMMAND_H
