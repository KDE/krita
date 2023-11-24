/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgInlineSizeChangeStrategy.h"
#include "SvgInlineSizeChangeCommand.h"
#include "SvgMoveTextCommand.h"
#include "SvgInlineSizeHelper.h"

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include "KoSvgTextShape.h"
#include "KoSnapGuide.h"

#include "KoCanvasBase.h"
#include "KoToolBase.h"
#include "kis_assert.h"
#include "kis_global.h"

using SvgInlineSizeHelper::InlineSizeInfo;
using SvgInlineSizeHelper::Side;
using SvgInlineSizeHelper::VisualAnchor;

SvgInlineSizeChangeStrategy::SvgInlineSizeChangeStrategy(KoToolBase *tool,
                                                         KoSvgTextShape *shape,
                                                         const QPointF &clicked, bool start)
    : KoInteractionStrategy(tool)
    , m_shape(shape)
    , m_dragStart(clicked)
    , m_initialPosition(shape->absolutePosition(KoFlake::TopLeft))
    , m_finalPos(m_initialPosition)
    , m_anchorOffset(m_shape->absoluteTransformation().map(QPointF()))
    , m_startHandle(start)
{
    this->tool()->canvas()->snapGuide()->setIgnoredShapes(KoShape::linearizeSubtree({shape}));
    m_originalAnchor = m_finalAnchor =
        KoSvgText::TextAnchor(shape->textProperties().propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());
    if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(shape)) {
        m_initialInlineSize = m_finalInlineSize = info->inlineSize;
        m_anchor = info->anchor;
        m_handleSide = m_startHandle? info->startLineSide(): info->endLineSide();
        QPointF handleLocation = m_startHandle? info->startLine().p1(): info->endLine().p1();
        QTransform invTransform = (info->editorTransform * info->shapeTransform).inverted();
        QPointF initPos = info->editorTransform.inverted().map(m_shape->initialTextPosition());
        m_snapDelta = invTransform.inverted().map(QPointF(invTransform.map(handleLocation).x(), initPos.y())) - m_dragStart;
    } else {
        // We cannot bail out, so just pretend to be doing something :(
        m_initialInlineSize = m_finalInlineSize = SvgInlineSizeHelper::getInlineSizePt(shape);
        m_anchor = VisualAnchor::LeftOrTop;
        m_handleSide = m_startHandle? Side::LeftOrTop: Side::RightOrBottom;
    }
}

void SvgInlineSizeChangeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QTransform invTransform{};
    QPointF initPos;
    if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(m_shape)) {
        invTransform = (info->editorTransform * info->shapeTransform).inverted();
        initPos = info->editorTransform.inverted().map(m_shape->initialTextPosition());
    }

    double newInlineSize = 0.0;

    QPointF snapDelta = invTransform.inverted().map(QPointF(invTransform.map(mouseLocation + m_snapDelta).x(), initPos.y())) - mouseLocation;
    QPointF snappedLocation = tool()->canvas()->snapGuide()->snap(mouseLocation + snapDelta, modifiers) - snapDelta;
    const double mouseDelta = invTransform.map(QLineF(m_dragStart, snappedLocation)).dx();
    QPointF newPosition = m_shape->absolutePosition(KoFlake::TopLeft);

    // The anchor pos is mostly to determine the transformed origin so that moving the position stays consistent.
    QPointF anchorPos = m_shape->absoluteTransformation().map(QPointF());
    QPointF anchorDiff = anchorPos - m_anchorOffset;
    QPointF diff = (invTransform.inverted().map(QPointF(mouseDelta, 0)) - anchorPos) - anchorDiff;


    switch (m_anchor) {
    case VisualAnchor::LeftOrTop:
        if (m_handleSide == Side::RightOrBottom) {
            newInlineSize = m_initialInlineSize + mouseDelta;
        } else {
            newInlineSize = m_initialInlineSize - mouseDelta;
        }
        break;
    case VisualAnchor::Mid:
        if (modifiers.testFlag(Qt::ControlModifier)) {
            if (m_handleSide == Side::RightOrBottom) {
                newInlineSize = m_initialInlineSize + 2.0 * mouseDelta;
            } else {
                newInlineSize = m_initialInlineSize - 2.0 * mouseDelta;
            }
            diff = QPointF();
            newPosition -= anchorDiff;
        } else {
            if (m_handleSide == Side::RightOrBottom) {
                newInlineSize = m_initialInlineSize + mouseDelta;
                newPosition += ((invTransform.inverted().map(QPointF(0.5 * mouseDelta, 0)) - anchorPos)) - anchorDiff;
            } else {
                newInlineSize = m_initialInlineSize - mouseDelta;
                diff = ((invTransform.inverted().map(QPointF(0.5 * mouseDelta, 0)) - anchorPos)) - anchorDiff;
            }
        }
        break;
    case VisualAnchor::RightOrBottom:
        if (m_handleSide == Side::LeftOrTop) {
            newInlineSize = m_initialInlineSize - mouseDelta;
        } else {
            newInlineSize = m_initialInlineSize + mouseDelta;
        }
        break;
    }

    if (m_startHandle) {
        newPosition += diff;
    }

    const bool flip = newInlineSize < -1.0;
    if (newInlineSize >= -1.0 && newInlineSize < 1.0) {
        newInlineSize = 1.0;
    } else {
        newInlineSize = qRound(newInlineSize * 100.0) / 100.0;

    }

    KoSvgText::TextAnchor newAnchor = KoSvgText::TextAnchor(m_originalAnchor);
    if (flip) {
        newInlineSize = fabs(newInlineSize);
        if (newAnchor == KoSvgText::AnchorStart) {
            newAnchor = KoSvgText::AnchorEnd;
        } else if (newAnchor == KoSvgText::AnchorEnd) {
            newAnchor = KoSvgText::AnchorStart;
        }
    }
    if (qFuzzyCompare(m_finalInlineSize, newInlineSize)
            && m_initialPosition == newPosition
            && m_originalAnchor == newAnchor) {
        return;
    }
    SvgInlineSizeChangeCommand(m_shape, newInlineSize, m_initialInlineSize, newAnchor, m_originalAnchor,
                               newPosition, m_initialPosition).redo();

    m_finalInlineSize = newInlineSize;
    m_finalAnchor = newAnchor;
    m_finalPos = newPosition;
    tool()->repaintDecorations();
}

KUndo2Command *SvgInlineSizeChangeStrategy::createCommand()
{
    tool()->canvas()->snapGuide()->reset();
    if (qFuzzyCompare(m_initialInlineSize, m_finalInlineSize)
            && m_initialPosition == m_finalPos
            && m_originalAnchor == m_finalAnchor) {
        return nullptr;
    }
    return new SvgInlineSizeChangeCommand(m_shape, m_finalInlineSize, m_initialInlineSize, m_finalAnchor,
                                          m_originalAnchor, m_finalPos, m_initialPosition);
}

void SvgInlineSizeChangeStrategy::cancelInteraction()
{
    if (qFuzzyCompare(m_initialInlineSize, m_finalInlineSize)
            && m_initialPosition == m_finalPos
            && m_originalAnchor == m_finalAnchor) {
        return;
    }
    SvgInlineSizeChangeCommand(m_shape, m_finalInlineSize, m_initialInlineSize, m_finalAnchor,
                               m_originalAnchor, m_finalPos, m_initialPosition).undo();
    tool()->repaintDecorations();
}

void SvgInlineSizeChangeStrategy::finishInteraction(Qt::KeyboardModifiers /*modifiers*/)
{
}
