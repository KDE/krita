/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "ShapeShearStrategy.h"
#include "SelectionDecorator.h"

#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <commands/KoShapeShearCommand.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeTransformCommand.h>

#include <KoSelection.h>
#include <QPointF>

#include <math.h>
#include <QDebug>
#include <klocalizedstring.h>

ShapeShearStrategy::ShapeShearStrategy(KoToolBase *tool, KoSelection *selection, const QPointF &clicked, KoFlake::SelectionHandle direction)
    : KoInteractionStrategy(tool)
    , m_start(clicked)
{
    /**
     * The outline of the selection should look as if it is also shear'ed, so we
     * add it to the transformed shapes list.
     */
    m_transformedShapesAndSelection = selection->selectedEditableShapes();
    m_transformedShapesAndSelection << selection;

    Q_FOREACH (KoShape *shape, m_transformedShapesAndSelection) {
        m_oldTransforms << shape->transformation();
    }

    // Even though we aren't currently activated by the corner handles we might as well code like it
    switch (direction) {
    case KoFlake::TopMiddleHandle:
        m_top = true; m_bottom = false; m_left = false; m_right = false; break;
    case KoFlake::TopRightHandle:
        m_top = true; m_bottom = false; m_left = false; m_right = true; break;
    case KoFlake::RightMiddleHandle:
        m_top = false; m_bottom = false; m_left = false; m_right = true; break;
    case KoFlake::BottomRightHandle:
        m_top = false; m_bottom = true; m_left = false; m_right = true; break;
    case KoFlake::BottomMiddleHandle:
        m_top = false; m_bottom = true; m_left = false; m_right = false; break;
    case KoFlake::BottomLeftHandle:
        m_top = false; m_bottom = true; m_left = true; m_right = false; break;
    case KoFlake::LeftMiddleHandle:
        m_top = false; m_bottom = false; m_left = true; m_right = false; break;
    case KoFlake::TopLeftHandle:
        m_top = true; m_bottom = false; m_left = true; m_right = false; break;
    default:
        ;// throw exception ?  TODO
    }
    m_initialSize = selection->size();
    m_solidPoint = QPointF(m_initialSize.width() / 2, m_initialSize.height() / 2);

    if (m_top) {
        m_solidPoint += QPointF(0, m_initialSize.height() / 2);
    } else if (m_bottom) {
        m_solidPoint -= QPointF(0, m_initialSize.height() / 2);
    }
    if (m_left) {
        m_solidPoint += QPointF(m_initialSize.width() / 2, 0);
    } else if (m_right) {
        m_solidPoint -= QPointF(m_initialSize.width() / 2, 0);
    }

    m_solidPoint = selection->absoluteTransformation().map(selection->outlineRect().topLeft() + m_solidPoint);

    QPointF edge;
    qreal angle = 0.0;
    if (m_top) {
        edge = selection->absolutePosition(KoFlake::BottomLeft) - selection->absolutePosition(KoFlake::BottomRight);
        angle = 180.0;
    } else if (m_bottom) {
        edge = selection->absolutePosition(KoFlake::TopRight) - selection->absolutePosition(KoFlake::TopLeft);
        angle = 0.0;
    } else if (m_left) {
        edge = selection->absolutePosition(KoFlake::BottomLeft) - selection->absolutePosition(KoFlake::TopLeft);
        angle = 90.0;
    } else if (m_right) {
        edge = selection->absolutePosition(KoFlake::TopRight) - selection->absolutePosition(KoFlake::BottomRight);
        angle = 270.0;
    }
    qreal currentAngle = atan2(edge.y(), edge.x()) / M_PI * 180;
    m_initialSelectionAngle = currentAngle - angle;

    // use cross product of top edge and left edge of selection bounding rect
    // to determine if the selection is mirrored
    QPointF top = selection->absolutePosition(KoFlake::TopRight) - selection->absolutePosition(KoFlake::TopLeft);
    QPointF left = selection->absolutePosition(KoFlake::BottomLeft) - selection->absolutePosition(KoFlake::TopLeft);
    m_isMirrored = (top.x() * left.y() - top.y() * left.x()) < 0.0;
}

void ShapeShearStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QPointF shearVector = point - m_start;

    QTransform m;
    m.rotate(-m_initialSelectionAngle);
    shearVector = m.map(shearVector);

    qreal shearX = 0, shearY = 0;

    if (m_top || m_left) {
        shearVector = - shearVector;
    }
    if (m_top || m_bottom) {
        shearX = shearVector.x() / m_initialSize.height();
    }
    if (m_left || m_right) {
        shearY = shearVector.y() / m_initialSize.width();
    }

    // if selection is mirrored invert the shear values
    if (m_isMirrored) {
        shearX *= -1.0;
        shearY *= -1.0;
    }

    QTransform matrix;
    matrix.translate(m_solidPoint.x(), m_solidPoint.y());
    matrix.rotate(m_initialSelectionAngle);
    matrix.shear(shearX, shearY);
    matrix.rotate(-m_initialSelectionAngle);
    matrix.translate(-m_solidPoint.x(), -m_solidPoint.y());

    QTransform applyMatrix = matrix * m_shearMatrix.inverted();

    Q_FOREACH (KoShape *shape, m_transformedShapesAndSelection) {
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->applyAbsoluteTransformation(applyMatrix);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
    }
    m_shearMatrix = matrix;
}

void ShapeShearStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

KUndo2Command *ShapeShearStrategy::createCommand()
{
    QList<QTransform> newTransforms;
    Q_FOREACH (KoShape *shape, m_transformedShapesAndSelection) {
        newTransforms << shape->transformation();
    }
    KoShapeTransformCommand *cmd = new KoShapeTransformCommand(m_transformedShapesAndSelection, m_oldTransforms, newTransforms);
    cmd->setText(kundo2_i18n("Shear"));
    return cmd;
}
