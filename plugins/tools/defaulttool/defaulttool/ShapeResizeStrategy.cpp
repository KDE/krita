/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#include "ShapeResizeStrategy.h"
#include "SelectionDecorator.h"

#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <commands/KoShapeResizeCommand.h>
#include <kis_command_utils.h>
#include <KoSnapGuide.h>
#include <KoToolBase.h>
#include <KoSelection.h>

#include <klocalizedstring.h>
#include <limits>
#include <math.h>

#include <kis_debug.h>

ShapeResizeStrategy::ShapeResizeStrategy(KoToolBase *tool, KoSelection *selection, const QPointF &clicked, KoFlake::SelectionHandle direction, bool forceUniformScalingMode)
    : KoInteractionStrategy(tool),
      m_forceUniformScalingMode(forceUniformScalingMode)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(selection && selection->count() > 0);

    m_selectedShapes = selection->selectedEditableShapes();
    m_start = clicked;

    KoShape *shape = 0;
    if (m_selectedShapes.size() > 1) {
        shape = selection;
    } else if (m_selectedShapes.size() == 1) {
        shape = m_selectedShapes.first();
    }

    if (shape) {
        const qreal w = shape->size().width();
        const qreal h = shape->size().height();

        switch (direction) {
        case KoFlake::TopMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::TopLeft) + shape->absolutePosition(KoFlake::TopRight));
            m_top = true; m_bottom = false; m_left = false; m_right = false;
            m_globalStillPoint = QPointF(0.5 * w, h);
            break;
        case KoFlake::TopRightHandle:
            m_start = shape->absolutePosition(KoFlake::TopRight);
            m_top = true; m_bottom = false; m_left = false; m_right = true;
            m_globalStillPoint = QPointF(0, h);
            break;
        case KoFlake::RightMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::TopRight) + shape->absolutePosition(KoFlake::BottomRight));
            m_top = false; m_bottom = false; m_left = false; m_right = true;
            m_globalStillPoint = QPointF(0, 0.5 * h);
            break;
        case KoFlake::BottomRightHandle:
            m_start = shape->absolutePosition(KoFlake::BottomRight);
            m_top = false; m_bottom = true; m_left = false; m_right = true;
            m_globalStillPoint = QPointF(0, 0);
            break;
        case KoFlake::BottomMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::BottomRight) + shape->absolutePosition(KoFlake::BottomLeft));
            m_top = false; m_bottom = true; m_left = false; m_right = false;
            m_globalStillPoint = QPointF(0.5 * w, 0);
            break;
        case KoFlake::BottomLeftHandle:
            m_start = shape->absolutePosition(KoFlake::BottomLeft);
            m_top = false; m_bottom = true; m_left = true; m_right = false;
            m_globalStillPoint = QPointF(w, 0);
            break;
        case KoFlake::LeftMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::BottomLeft) + shape->absolutePosition(KoFlake::TopLeft));
            m_top = false; m_bottom = false; m_left = true; m_right = false;
            m_globalStillPoint = QPointF(w, 0.5 * h);
            break;
        case KoFlake::TopLeftHandle:
            m_start = shape->absolutePosition(KoFlake::TopLeft);
            m_top = true; m_bottom = false; m_left = true; m_right = false;
            m_globalStillPoint = QPointF(w, h);
            break;
        default:
            Q_ASSERT(0); // illegal 'corner'
        }

        const QPointF p0 = shape->outlineRect().topLeft();
        m_globalStillPoint = shape->absoluteTransformation(0).map(p0 + m_globalStillPoint);
        m_globalCenterPoint = shape->absolutePosition(KoFlake::Center);

        m_unwindMatrix = shape->absoluteTransformation(0).inverted();
        m_initialSelectionSize = shape->size();
        m_postScalingCoveringTransform = shape->transformation();
    }

    tool->setStatusText(i18n("Press CTRL to resize from center."));
    tool->canvas()->snapGuide()->setIgnoredShapes(KoShape::linearizeSubtree(m_selectedShapes));
}

ShapeResizeStrategy::~ShapeResizeStrategy()
{

}

void ShapeResizeStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
    QPointF newPos = tool()->canvas()->snapGuide()->snap(point, modifiers);
    tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());

    bool keepAspect = modifiers & Qt::ShiftModifier;
    Q_FOREACH (KoShape *shape, m_selectedShapes) {
        keepAspect = keepAspect || shape->keepAspectRatio();
    }

    qreal startWidth = m_initialSelectionSize.width();
    if (startWidth < std::numeric_limits<qreal>::epsilon()) {
        startWidth = std::numeric_limits<qreal>::epsilon();
    }
    qreal startHeight = m_initialSelectionSize.height();
    if (startHeight < std::numeric_limits<qreal>::epsilon()) {
        startHeight = std::numeric_limits<qreal>::epsilon();
    }

    QPointF distance = m_unwindMatrix.map(newPos) - m_unwindMatrix.map(m_start);

    // guard against resizing zero width shapes, which would result in huge zoom factors
    if (m_initialSelectionSize.width() < std::numeric_limits<qreal>::epsilon()) {
        distance.rx() = 0.0;
    }
    // guard against resizing zero height shapes, which would result in huge zoom factors
    if (m_initialSelectionSize.height() < std::numeric_limits<qreal>::epsilon()) {
        distance.ry() = 0.0;
    }

    const bool scaleFromCenter = modifiers & Qt::ControlModifier;
    if (scaleFromCenter) {
        distance *= 2.0;
    }

    qreal newWidth = startWidth;
    qreal newHeight = startHeight;

    if (m_left) {
        newWidth = startWidth - distance.x();
    } else if (m_right) {
        newWidth = startWidth + distance.x();
    }

    if (m_top) {
        newHeight = startHeight - distance.y();
    } else if (m_bottom) {
        newHeight = startHeight + distance.y();
    }

    /**
     * Do not let a shape be less than 1px in size in current view
     * coordinates.  If the user wants it to be smaller, he can just
     * zoom-in a bit.
     */
    QSizeF minViewSize(1.0, 1.0);
    QSizeF minDocSize = tool()->canvas()->viewConverter()->viewToDocument(minViewSize);

    if (qAbs(newWidth) < minDocSize.width()) {
        int sign = newWidth >= 0.0 ? 1 : -1; // zero -> '1'
        newWidth = sign * minDocSize.width();
    }

    if (qAbs(newHeight) < minDocSize.height()) {
        int sign = newHeight >= 0.0 ? 1 : -1; // zero -> '1'
        newHeight = sign * minDocSize.height();
    }

    qreal zoomX = newWidth / startWidth;
    qreal zoomY = newHeight / startHeight;

    if (keepAspect) {
        const bool cornerUsed = ((m_bottom ? 1 : 0) + (m_top ? 1 : 0) + (m_left ? 1 : 0) + (m_right ? 1 : 0)) == 2;
        if (cornerUsed) {
            if (startWidth < startHeight) {
                zoomY = zoomX;
            } else {
                zoomX = zoomY;
            }
        } else {
            if (m_left || m_right) {
               zoomY = qAbs(zoomX);
            } else {
               zoomX = qAbs(zoomY);
            }
        }
    }

    resizeBy(scaleFromCenter ? m_globalCenterPoint : m_globalStillPoint, zoomX, zoomY);
}

void ShapeResizeStrategy::resizeBy(const QPointF &stillPoint, qreal zoomX, qreal zoomY)
{
    if (!m_executedCommand) {
        const bool usePostScaling = m_selectedShapes.size() > 1 || m_forceUniformScalingMode;

        m_executedCommand.reset(
                    new KoShapeResizeCommand(
                        m_selectedShapes,
                        zoomX, zoomY,
                        stillPoint,
                        false, usePostScaling, m_postScalingCoveringTransform));
        m_executedCommand->redo();
    } else {
        m_executedCommand->replaceResizeAction(zoomX, zoomY, stillPoint);
    }
}

KUndo2Command *ShapeResizeStrategy::createCommand()
{
    tool()->canvas()->snapGuide()->reset();

    if (m_executedCommand) {
        m_executedCommand->setSkipOneRedo(true);
    }

    return m_executedCommand.take();
}

void ShapeResizeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
}

void ShapeResizeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}
