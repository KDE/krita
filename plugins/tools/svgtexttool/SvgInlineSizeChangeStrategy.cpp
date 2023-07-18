/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgInlineSizeChangeStrategy.h"
#include "SvgInlineSizeChangeCommand.h"
#include "SvgInlineSizeHelper.h"

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"

#include "KoCanvasBase.h"
#include "KoToolBase.h"
#include "kis_assert.h"
#include "kis_global.h"

using SvgInlineSizeHelper::InlineSizeInfo;
using SvgInlineSizeHelper::Side;
using SvgInlineSizeHelper::VisualAnchor;

SvgInlineSizeChangeStrategy::SvgInlineSizeChangeStrategy(KoToolBase *tool,
                                                         KoSvgTextShape *shape,
                                                         const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_shape(shape)
    , m_dragStart(clicked)
{
    if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(shape)) {
        m_initialInlineSize = m_finalInlineSize = info->inlineSize;
        m_anchor = info->anchor;
        m_handleSide = info->editLineSide();
    } else {
        // We cannot bail out, so just pretend to be doing something :(
        m_initialInlineSize = m_finalInlineSize = SvgInlineSizeHelper::getInlineSizePt(shape);
        m_anchor = VisualAnchor::LeftOrTop;
        m_handleSide = Side::RightOrBottom;
    }
}

void SvgInlineSizeChangeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers /*modifiers*/)
{
    QRectF updateRect = m_shape->boundingRect();
    QTransform invTransform{};
    if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(m_shape)) {
        updateRect |= info->boundingRect();
        invTransform = info->absTransform.inverted();
    }

    double newInlineSize = 0.0;
    const double mouseDelta = invTransform.map(QLineF(m_dragStart, mouseLocation)).dx();
    switch (m_anchor) {
    case VisualAnchor::LeftOrTop:
        if (m_handleSide == Side::RightOrBottom) {
            newInlineSize = m_initialInlineSize + mouseDelta;
        }
        break;
    case VisualAnchor::Mid:
        if (m_handleSide == Side::RightOrBottom) {
            newInlineSize = m_initialInlineSize + 2.0 * mouseDelta;
        }
        break;
    case VisualAnchor::RightOrBottom:
        if (m_handleSide == Side::LeftOrTop) {
            newInlineSize = m_initialInlineSize - mouseDelta;
        }
        break;
    }
    if (newInlineSize < 1.0) {
        newInlineSize = m_initialInlineSize;
    } else {
        newInlineSize = qRound(newInlineSize * 100.0) / 100.0;
    }
    if (qFuzzyCompare(m_finalInlineSize, newInlineSize)) {
        return;
    }
    SvgInlineSizeChangeCommand(m_shape, newInlineSize, m_initialInlineSize).redo();

    m_finalInlineSize = newInlineSize;

    updateRect |= m_shape->boundingRect();
    if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(m_shape)) {
        updateRect |= info->boundingRect();
    }
    tool()->canvas()->updateCanvas(kisGrowRect(updateRect, 100));
}

KUndo2Command *SvgInlineSizeChangeStrategy::createCommand()
{
    if (qFuzzyCompare(m_initialInlineSize, m_finalInlineSize)) {
        return nullptr;
    }
    return new SvgInlineSizeChangeCommand(m_shape, m_finalInlineSize, m_initialInlineSize);
}

void SvgInlineSizeChangeStrategy::cancelInteraction()
{
    if (qFuzzyCompare(m_initialInlineSize, m_finalInlineSize)) {
        return;
    }
    SvgInlineSizeChangeCommand(m_shape, m_finalInlineSize, m_initialInlineSize).undo();
}

void SvgInlineSizeChangeStrategy::finishInteraction(Qt::KeyboardModifiers /*modifiers*/)
{
}
