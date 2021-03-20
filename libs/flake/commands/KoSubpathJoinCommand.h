/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSUBPATHJOINCOMMAND_H
#define KOSUBPATHJOINCOMMAND_H

#include <kundo2command.h>
#include <QPointF>
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include <boost/optional.hpp>

/// The undo / redo command for joining two subpath end points
class KoSubpathJoinCommand : public KUndo2Command
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
    KoSubpathJoinCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, KUndo2Command *parent = 0);
    ~KoSubpathJoinCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    bool closeSubpathMode() const;

private:
    KoPathPointData m_pointData1;
    KoPathPointData m_pointData2;
    KoPathPointIndex m_splitIndex;

    // the control points have to be stored in shape coordinates
    boost::optional<QPointF> m_savedControlPoint1;
    boost::optional<QPointF> m_savedControlPoint2;

    KoPathPoint::PointProperties m_oldProperties1;
    KoPathPoint::PointProperties m_oldProperties2;
    enum Reverse {
        ReverseFirst = 1,
        ReverseSecond = 2
    };
    int m_reverse;
};

#endif // KOSUBPATHJOINCOMMAND_H
