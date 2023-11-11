/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgMoveTextStrategy.h"
#include "SvgMoveTextCommand.h"

#include "KoSvgText.h"
#include "KoSvgTextShape.h"

#include "KoCanvasBase.h"
#include "KoSnapGuide.h"
#include "KoToolBase.h"
#include "kis_algebra_2d.h"

SvgMoveTextStrategy::SvgMoveTextStrategy(KoToolBase *tool, KoSvgTextShape *shape, const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_shape(shape)
    , m_dragStart(clicked)
    , m_initialPosition(shape->absolutePosition())
    , m_finalPosition(m_initialPosition)
    , m_anchorOffset(m_shape->absoluteTransformation().map(QPointF()) - m_initialPosition)
{
    this->tool()->canvas()->snapGuide()->setIgnoredShapes(KoShape::linearizeSubtree({shape}));
}

void SvgMoveTextStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    const QPointF delta = mouseLocation - m_dragStart;

    if (modifiers & Qt::ShiftModifier) {
        m_finalPosition = m_initialPosition+ snapToClosestAxis(delta);
    } else {
        m_finalPosition =
            tool()->canvas()->snapGuide()->snap(m_initialPosition + m_anchorOffset + delta, modifiers) - m_anchorOffset;
    }

    SvgMoveTextCommand(m_shape, m_finalPosition, m_initialPosition).redo();
    tool()->repaintDecorations();
}

KUndo2Command *SvgMoveTextStrategy::createCommand()
{
    tool()->canvas()->snapGuide()->reset();
    if (KisAlgebra2D::fuzzyPointCompare(m_initialPosition, m_finalPosition)) {
        return nullptr;
    }
    return new SvgMoveTextCommand(m_shape, m_finalPosition, m_initialPosition);
}

void SvgMoveTextStrategy::cancelInteraction()
{
    SvgMoveTextCommand(m_shape, m_finalPosition, m_initialPosition).undo();
    tool()->repaintDecorations();
}

void SvgMoveTextStrategy::finishInteraction(Qt::KeyboardModifiers /*modifiers*/)
{
}
