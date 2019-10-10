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

#include "KoPathControlPointMoveCommand.h"
#include <klocalizedstring.h>
#include <math.h>
#include "kis_command_ids.h"

KoPathControlPointMoveCommand::KoPathControlPointMoveCommand(
    const KoPathPointData &pointData,
    const QPointF &offset,
    KoPathPoint::PointType pointType,
    KUndo2Command *parent)
        : KUndo2Command(parent)
        , m_pointData(pointData)
        , m_pointType(pointType)
{
    Q_ASSERT(offset.x() < 1e14 && offset.y() < 1e14);
    KoPathShape * pathShape = m_pointData.pathShape;
    KoPathPoint * point = pathShape->pointByIndex(m_pointData.pointIndex);
    if (point) {
        m_offset = point->parent()->documentToShape(offset) - point->parent()->documentToShape(QPointF(0, 0));
    }

    setText(kundo2_i18n("Move control point"));
}

void KoPathControlPointMoveCommand::redo()
{
    KUndo2Command::redo();
    KoPathShape * pathShape = m_pointData.pathShape;
    KoPathPoint * point = pathShape->pointByIndex(m_pointData.pointIndex);
    if (point) {
        const QRectF oldDirtyRect = pathShape->boundingRect();

        if (m_pointType == KoPathPoint::ControlPoint1) {
            point->setControlPoint1(point->controlPoint1() + m_offset);
            if (point->properties() & KoPathPoint::IsSymmetric) {
                // set the other control point so that it lies on the line between the moved
                // control point and the point, with the same distance to the point as the moved point
                point->setControlPoint2(2.0 * point->point() - point->controlPoint1());
            } else if (point->properties() & KoPathPoint::IsSmooth) {
                // move the other control point so that it lies on the line through point and control point
                // keeping its distance to the point
                QPointF direction = point->point() - point->controlPoint1();
                direction /= sqrt(direction.x() * direction.x() + direction.y() * direction.y());
                QPointF distance = point->point() - point->controlPoint2();
                qreal length = sqrt(distance.x() * distance.x() + distance.y() * distance.y());
                point->setControlPoint2(point->point() + length * direction);
            }
        } else if (m_pointType == KoPathPoint::ControlPoint2) {
            point->setControlPoint2(point->controlPoint2() + m_offset);
            if (point->properties() & KoPathPoint::IsSymmetric) {
                // set the other control point so that it lies on the line between the moved
                // control point and the point, with the same distance to the point as the moved point
                point->setControlPoint1(2.0 * point->point() - point->controlPoint2());
            } else if (point->properties() & KoPathPoint::IsSmooth) {
                // move the other control point so that it lies on the line through point and control point
                // keeping its distance to the point
                QPointF direction = point->point() - point->controlPoint2();
                direction /= sqrt(direction.x() * direction.x() + direction.y() * direction.y());
                QPointF distance = point->point() - point->controlPoint1();
                qreal length = sqrt(distance.x() * distance.x() + distance.y() * distance.y());
                point->setControlPoint1(point->point() + length * direction);
            }
        }

        pathShape->normalize();
        pathShape->updateAbsolute(oldDirtyRect | pathShape->boundingRect());
    }
}

void KoPathControlPointMoveCommand::undo()
{
    KUndo2Command::undo();
    m_offset *= -1.0;
    redo();
    m_offset *= -1.0;
}

int KoPathControlPointMoveCommand::id() const
{
    return KisCommandUtils::ChangePathShapeControlPointId;
}

bool KoPathControlPointMoveCommand::mergeWith(const KUndo2Command *command)
{
    const KoPathControlPointMoveCommand *other = dynamic_cast<const KoPathControlPointMoveCommand*>(command);

    if (!other ||
        other->m_pointData != m_pointData ||
        other->m_pointType != m_pointType) {

        return false;
    }

    m_offset += other->m_offset;

    return true;
}
