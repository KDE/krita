/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgTextTypeSettingStrategy.h"
#include "SvgTextCursor.h"
#include "SvgTextChangeTransformsOnRange.h"
#include "SvgTextShapeManagerBlocker.h"

#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include "KoSnapGuide.h"
#include <kis_algebra_2d.h>
#include <QDebug>
#include <KoViewConverter.h>

SvgTextTypeSettingStrategy::SvgTextTypeSettingStrategy(KoToolBase *tool, KoSvgTextShape *textShape, SvgTextCursor *textCursor, const QRectF &regionOfInterest)
    : KoInteractionStrategy(tool)
    , m_shape(textShape)
    , m_dragStart(regionOfInterest.center())
{

    m_cursorPos = textCursor->getPos();
    m_cursorAnchor = textCursor->getAnchor();
    qDebug() << Q_FUNC_INFO << regionOfInterest << m_cursorPos << m_cursorAnchor;
    m_editingType = textCursor->typeSettingHandleAtPos(regionOfInterest);
}

void SvgTextTypeSettingStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    /*
    if (!m_shape) return;
    const int lineStart = qMax(m_shape->lineStart(m_cursorAnchor), qMin(m_cursorPos, m_cursorAnchor));
    QList<QPair<QPointF, qreal>> firstTf = m_shape->getPositionsAndRotationsForRange(lineStart, qMax(m_cursorPos, m_cursorAnchor), false);
    if (firstTf.size() < 2) return;
    QLineF originalLine(firstTf.first().first, firstTf.last().first);
    QPointF delta = m_dragCurrent - m_dragStart;
    QLineF newLine(originalLine.p1(), originalLine.p2() + delta);
    painter.save();
    painter.setPen(Qt::blue);
    painter.setTransform(converter.documentToView(), true);
    painter.drawPoint(m_dragStart);
    painter.drawPoint(m_dragCurrent);
    painter.setTransform(m_shape->absoluteTransformation(), true);
    painter.setPen(Qt::red);
    painter.setOpacity(0.3);
    painter.drawPoint(originalLine.p1());
    painter.drawLine(originalLine);
    painter.drawLine(newLine);
    painter.restore();
    */
}

void SvgTextTypeSettingStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF delta = mouseLocation - m_dragStart;

    if (modifiers & Qt::ShiftModifier) {
        delta = snapToClosestAxis(delta);
        m_dragCurrent = m_dragStart + delta;
        m_currentDelta = delta;
    } else {
        m_dragCurrent =
            tool()->canvas()->snapGuide()->snap(mouseLocation, modifiers);
        m_currentDelta = m_dragCurrent - m_dragStart;
    }

    if (m_editingType != int(SvgTextCursor::NoHandle)) {
        SvgTextShapeManagerBlocker blocker(tool()->canvas()->shapeManager());
        QRectF updateRect = m_shape->boundingRect();
        if (m_previousCmd) {
            m_previousCmd->undo();
        }
        m_previousCmd.reset(createCommand());
        if (m_previousCmd) {
            m_previousCmd->redo();
        }
        updateRect |= m_shape->boundingRect();
        blocker.unlock();
        m_shape->updateAbsolute(updateRect);
    }
}

KUndo2Command *SvgTextTypeSettingStrategy::createCommand()
{
    if (m_editingType == int(SvgTextCursor::NoHandle)) return nullptr;
    QPointF delta = m_currentDelta;

    QList<KoSvgTextCharacterInfo> originalTf = m_shape->getPositionsAndRotationsForRange(m_cursorPos, m_cursorAnchor);
    if (originalTf.isEmpty()) return nullptr;

    KUndo2Command *cmd = nullptr;
    if (m_editingType == int(SvgTextCursor::StartPos) || m_editingType == int(SvgTextCursor::EndPos)) {
        if (m_shape->textType() != KoSvgTextShape::PreformattedText && m_shape->textType() != KoSvgTextShape::PrePositionedText) return nullptr;
        SvgTextChangeTransformsOnRange::OffsetType type = m_editingType == int(SvgTextCursor::StartPos)? SvgTextChangeTransformsOnRange::OffsetAll: SvgTextChangeTransformsOnRange::ScaleAndRotate;

        cmd = new SvgTextChangeTransformsOnRange(m_shape, m_cursorPos, m_cursorAnchor, delta, type, true, nullptr);
    }
    return cmd;
}

void SvgTextTypeSettingStrategy::cancelInteraction()
{
    if (m_previousCmd) {
        m_previousCmd->undo();
    }
    tool()->repaintDecorations();
}

void SvgTextTypeSettingStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    if (m_previousCmd) {
        m_previousCmd->undo();
    }
    tool()->repaintDecorations();
}
