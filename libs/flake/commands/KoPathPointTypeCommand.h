/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHPOINTTYPECOMMAND_H
#define KOPATHPOINTTYPECOMMAND_H

#include <kundo2command.h>
#include <QList>
#include "KoPathBaseCommand.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "kritaflake_export.h"

/// The undo / redo command for changing the path point type.
class KRITAFLAKE_EXPORT KoPathPointTypeCommand : public KoPathBaseCommand
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
    KoPathPointTypeCommand(const QList<KoPathPointData> &pointDataList, PointType pointType, KUndo2Command *parent = 0);
    ~KoPathPointTypeCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    static void makeCubicPointSmooth(KoPathPoint *point);

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
        bool m_hadControlPoint1 {false};
        bool m_hadControlPoint2 {false};
    };

    bool appendPointData(KoPathPointData data);
    void undoChanges(const QList<PointData> &data);

    PointType m_pointType;
    QList<PointData> m_oldPointData;
    QList<PointData> m_additionalPointData;
};

#endif // KOPATHPOINTTYPECOMMAND_H
