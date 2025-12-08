/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgSelectTextStrategy.h"
#include "SvgTextCursor.h"
#include <QDebug>

SvgSelectTextStrategy::SvgSelectTextStrategy(KoToolBase *tool, SvgTextCursor *cursor, const QPointF &clicked, Qt::KeyboardModifiers modifiers)
    : KoInteractionStrategy(tool)
    , m_cursor(cursor)
    , m_dragStart(clicked)
{
    m_dragEnd = m_dragStart;
    m_cursor->setPosToPoint(m_dragStart, !(modifiers & Qt::ShiftModifier));
}

void SvgSelectTextStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    m_dragEnd = mouseLocation;
    if (!(modifiers & Qt::ShiftModifier)) {
        m_cursor->setPosToPoint(m_dragStart, true);
    }
    m_cursor->setPosToPoint(m_dragEnd, false);
}

KUndo2Command *SvgSelectTextStrategy::createCommand()
{
    return nullptr;
}

void SvgSelectTextStrategy::cancelInteraction()
{
    return;
}

void SvgSelectTextStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    if (!(modifiers & Qt::ShiftModifier)) {
        m_cursor->setPosToPoint(m_dragStart, true);
    }
    m_cursor->setPosToPoint(m_dragEnd, false);

}
