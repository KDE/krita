/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <m.Kruisselbrink@student.tue.nl>
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
#include "AbstractMusicAction.h"
#include "../SimpleEntryTool.h"

#include <QPainter>

AbstractMusicAction::AbstractMusicAction(const KIcon& icon, const QString& text, SimpleEntryTool* tool)
    : KAction(icon, text, tool)
    , m_isVoiceAware(false)
    , m_tool(tool)
{
    setCheckable(true);
}

AbstractMusicAction::AbstractMusicAction(const QString& text, SimpleEntryTool* tool)
    : KAction(text, tool)
    , m_isVoiceAware(false)
    , m_tool(tool)
{
    setCheckable(true);
}


void AbstractMusicAction::renderPreview(QPainter& painter, const QPointF& point)
{
    Q_UNUSED( painter );
    Q_UNUSED( point );
}

bool AbstractMusicAction::isVoiceAware()
{
    return m_isVoiceAware;
}

void AbstractMusicAction::mouseMove(MusicCore::Staff*, int, const QPointF&)
{
}

void AbstractMusicAction::renderKeyboardPreview(QPainter& painter, const MusicCursor& /*cursor*/)
{
    Q_UNUSED(painter);
}

void AbstractMusicAction::keyPress(QKeyEvent* event, const MusicCursor& /*cursor*/)
{
    Q_UNUSED(event);
}

#include <AbstractMusicAction.moc>
