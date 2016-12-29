/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
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
#include <commands/KoShapeSizeCommand.h>
#include <commands/KoShapeTransformCommand.h>
#include <KoSnapGuide.h>
#include <KoToolBase.h>
#include <KoSelection.h>

#include <klocalizedstring.h>
#include <limits>
#include <math.h>

#include <kis_debug.h>

ShapeResizeStrategy::ShapeResizeStrategy(KoToolBase *tool, const QPointF &clicked, KoFlake::SelectionHandle direction)
    : KoInteractionStrategy(tool)
{
    Q_ASSERT(tool->canvas()->shapeManager()->selection()->count() > 0);
    QList<KoShape *> selectedShapes = tool->canvas()->shapeManager()->selection()->selectedShapes();
    Q_FOREACH (KoShape *shape, selectedShapes) {
        if (!shape->isEditable()) {
            continue;
        }
        m_selectedShapes << shape;
        m_initialTransforms << shape->transformation();
        m_initialSizes << shape->size();
    }
    m_start = clicked;

   KoShape *shape = 0;
    if (m_selectedShapes.size() > 1) {
        shape = tool->canvas()->shapeManager()->selection();
    } else if (m_selectedShapes.size() == 1) {
        shape = m_selectedShapes.first();
    }

    if (shape) {
        const qreal w = shape->size().width();
        const qreal h = shape->size().height();

        switch (direction) {
        case KoFlake::TopMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::TopLeftCorner) + shape->absolutePosition(KoFlake::TopRightCorner));
            m_top = true; m_bottom = false; m_left = false; m_right = false;
            m_globalStillPoint = QPointF(0.5 * w, h);
            break;
        case KoFlake::TopRightHandle:
            m_start = shape->absolutePosition(KoFlake::TopRightCorner);
            m_top = true; m_bottom = false; m_left = false; m_right = true;
            m_globalStillPoint = QPointF(0, h);
            break;
        case KoFlake::RightMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::TopRightCorner) + shape->absolutePosition(KoFlake::BottomRightCorner));
            m_top = false; m_bottom = false; m_left = false; m_right = true;
            m_globalStillPoint = QPointF(0, 0.5 * h);
            break;
        case KoFlake::BottomRightHandle:
            m_start = shape->absolutePosition(KoFlake::BottomRightCorner);
            m_top = false; m_bottom = true; m_left = false; m_right = true;
            m_globalStillPoint = QPointF(0, 0);
            break;
        case KoFlake::BottomMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::BottomRightCorner) + shape->absolutePosition(KoFlake::BottomLeftCorner));
            m_top = false; m_bottom = true; m_left = false; m_right = false;
            m_globalStillPoint = QPointF(0.5 * w, 0);
            break;
        case KoFlake::BottomLeftHandle:
            m_start = shape->absolutePosition(KoFlake::BottomLeftCorner);
            m_top = false; m_bottom = true; m_left = true; m_right = false;
            m_globalStillPoint = QPointF(w, 0);
            break;
        case KoFlake::LeftMiddleHandle:
            m_start = 0.5 * (shape->absolutePosition(KoFlake::BottomLeftCorner) + shape->absolutePosition(KoFlake::TopLeftCorner));
            m_top = false; m_bottom = false; m_left = true; m_right = false;
            m_globalStillPoint = QPointF(w, 0.5 * h);
            break;
        case KoFlake::TopLeftHandle:
            m_start = shape->absolutePosition(KoFlake::TopLeftCorner);
            m_top = true; m_bottom = false; m_left = true; m_right = false;
            m_globalStillPoint = QPointF(w, h);
            break;
        default:
            Q_ASSERT(0); // illegal 'corner'
        }

        const QPointF p0 = shape->outlineRect().topLeft();
        m_globalStillPoint = shape->absoluteTransformation(0).map(p0 + m_globalStillPoint);
        m_globalCenterPoint = shape->absolutePosition(KoFlake::CenteredPosition);

        m_unwindMatrix = shape->absoluteTransformation(0).inverted();
        m_initialSelectionSize = shape->size();
        m_postScalingCoveringTransform = shape->transformation();
    }

    tool->setStatusText(i18n("Press CTRL to resize from center."));
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
        if ((cornerUsed && startWidth < startHeight) || m_left || m_right) {
            zoomY = zoomX;
        } else {
            zoomX = zoomY;
        }
    }

    resizeBy(scaleFromCenter ? m_globalCenterPoint : m_globalStillPoint, zoomX, zoomY);
}

void ShapeResizeStrategy::resizeBy(const QPointF &stillPoint, qreal zoomX, qreal zoomY)
{
    const bool usePostScaling = m_selectedShapes.size() > 1;

    int i = 0;
    Q_FOREACH (KoShape *shape, m_selectedShapes) {
        shape->update();

        shape->setTransformation(m_initialTransforms[i]);
        shape->setSize(m_initialSizes[i]);
        KoFlake::resizeShape(shape, zoomX, zoomY,
                             stillPoint,
                             usePostScaling, m_postScalingCoveringTransform);

        shape->update();
        i++;
    }
}

KUndo2Command *ShapeResizeStrategy::createCommand()
{
    tool()->canvas()->snapGuide()->reset();
    QList<QSizeF> newSizes;
    QList<QTransform> transformations;
    const int shapeCount = m_selectedShapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        newSizes << m_selectedShapes[i]->size();
        transformations << m_selectedShapes[i]->transformation();
    }
    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Resize"));
    new KoShapeSizeCommand(m_selectedShapes, m_initialSizes, newSizes, cmd);
    new KoShapeTransformCommand(m_selectedShapes, m_initialTransforms, transformations, cmd);
    return cmd;
}

void ShapeResizeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
}

void ShapeResizeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    SelectionDecorator decorator(tool()->canvas()->resourceManager());
    decorator.setSelection(tool()->canvas()->shapeManager()->selection());
    decorator.setHandleRadius(handleRadius());
    decorator.paint(painter, converter);
}
