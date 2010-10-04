/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathSegmentChangeStrategy.h"
#include "KoPathShape.h"
#include "KoPathPoint.h"
#include "KoPathTool.h"
#include "KoSnapGuide.h"
#include "commands/KoPathControlPointMoveCommand.h"
#include "commands/KoPathSegmentTypeCommand.h"
#include <KoCanvasBase.h>
#include <KLocale>
#include <limits>

KoPathSegmentChangeStrategy::KoPathSegmentChangeStrategy(KoPathTool *tool, const QPointF &pos, const KoPathPointData &segment, qreal segmentParam)
: KoInteractionStrategy(tool)
, m_originalPosition(pos)
, m_lastPosition(pos)
, m_tool(tool)
, m_segmentParam(segmentParam)
, m_pointData1(segment)
, m_pointData2(segment)
{
    // The following value is a bit arbitrary, it would be more mathematically correct to use
    // "std::numeric_limits<qreal>::epsilon()", but if the value is too small, when the user
    // click near a control point it is relatively easy to create a path shape of almost
    // infinite size, which blocks the application for a long period of time. A bigger value
    // is mathematically uncorrect, but it avoids to block application, it also avoid to create
    // an huge path shape by accident, and anyway, but it does not prevent the user to create one
    // if he choose so.
    const qreal eps = 1e-2;
    // force segment parameter range to avoid division by zero
    m_segmentParam = qBound(eps, m_segmentParam, qreal(1.0)-eps);

    m_path = segment.pathShape;
    m_segment = m_path->segmentByIndex(segment.pointIndex);
    m_pointData2.pointIndex = m_path->pathPointIndex(m_segment.second());
    m_originalSegmentDegree = m_segment.degree();
}

KoPathSegmentChangeStrategy::~KoPathSegmentChangeStrategy()
{
}

void KoPathSegmentChangeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_tool->canvas()->updateCanvas(m_tool->canvas()->snapGuide()->boundingRect());
    QPointF snappedPosition = m_tool->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    m_tool->canvas()->updateCanvas(m_tool->canvas()->snapGuide()->boundingRect());

    QPointF localPos = m_path->documentToShape(snappedPosition);

    if (m_segment.degree() == 1) {
        // line segment is converted to a curve
        KoPathSegmentTypeCommand cmd(m_pointData1, KoPathSegmentTypeCommand::Curve);
        cmd.redo();
    }

    QPointF move1, move2;

    if (m_segment.degree() == 2) {
        // interpolate quadratic segment between segment start, mouse position and segment end
        KoPathSegment ipol = KoPathSegment::interpolate(m_segment.first()->point(),
                                                         localPos,
                                                         m_segment.second()->point(),
                                                         m_segmentParam);
        if (ipol.isValid()) {
            move1 = move2 = ipol.controlPoints()[1] - m_segment.controlPoints()[1];
        }
    }
    else if (m_segment.degree() == 3) {
        /*
        * method from inkscape, original method and idea borrowed from Simon Budig
        * <simon@gimp.org> and the GIMP
        * cf. app/vectors/gimpbezierstroke.c, gimp_bezier_stroke_point_move_relative()
        *
        * feel good is an arbitrary parameter that distributes the delta between handles
        * if t of the drag point is less than 1/6 distance form the endpoint only
        * the corresponding handle is adjusted. This matches the behavior in GIMP
        */
        const qreal t = m_segmentParam;
        qreal feel_good;
        if (t <= 1.0 / 6.0)
            feel_good = 0;
        else if (t <= 0.5)
            feel_good = (pow((6 * t - 1) / 2.0, 3)) / 2;
        else if (t <= 5.0 / 6.0)
            feel_good = (1 - pow((6 * (1-t) - 1) / 2.0, 3)) / 2 + 0.5;
        else
            feel_good = 1;

        QPointF lastLocalPos = m_path->documentToShape(m_lastPosition);
        QPointF delta = localPos - lastLocalPos;
        move2 = ((1-feel_good)/(3*t*(1-t)*(1-t))) * delta;
        move1 = (feel_good/(3*t*t*(1-t))) * delta;
    }

    m_path->update();
    if(m_segment.first()->activeControlPoint2()) {
        KoPathControlPointMoveCommand cmd(m_pointData1, move2, KoPathPoint::ControlPoint2);
        cmd.redo();
    }
    if(m_segment.second()->activeControlPoint1()) {
        KoPathControlPointMoveCommand cmd(m_pointData2, move1, KoPathPoint::ControlPoint1);
        cmd.redo();
    }
    m_path->normalize();
    m_path->update();

    m_ctrlPoint1Move += move1;
    m_ctrlPoint2Move += move2;

    // save last mouse position
    m_lastPosition = mouseLocation;
}

void KoPathSegmentChangeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

QUndoCommand* KoPathSegmentChangeStrategy::createCommand()
{
    m_tool->canvas()->updateCanvas(m_tool->canvas()->snapGuide()->boundingRect());

    bool hasControlPoint1 = m_segment.second()->activeControlPoint1();
    bool hasControlPoint2 = m_segment.first()->activeControlPoint2();

    QUndoCommand * cmd = new QUndoCommand(i18n("Change Segment"));
    if (m_originalSegmentDegree == 1) {
        m_segment.first()->removeControlPoint2();
        m_segment.second()->removeControlPoint1();
        new KoPathSegmentTypeCommand(m_pointData1, KoPathSegmentTypeCommand::Curve, cmd);
    }

    if (hasControlPoint2) {
        QPointF oldCtrlPointPos = m_segment.first()->controlPoint2()-m_ctrlPoint2Move;
        m_segment.first()->setControlPoint2(oldCtrlPointPos);
        new KoPathControlPointMoveCommand(m_pointData1, m_ctrlPoint2Move, KoPathPoint::ControlPoint2, cmd);
    }
    if (hasControlPoint1) {
        QPointF oldCtrlPointPos = m_segment.second()->controlPoint1()-m_ctrlPoint1Move;
        m_segment.second()->setControlPoint1(oldCtrlPointPos);
        new KoPathControlPointMoveCommand(m_pointData2, m_ctrlPoint1Move, KoPathPoint::ControlPoint1, cmd);
    }

    return cmd;
}
