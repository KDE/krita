/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#include "ChangeTextFontCommand.h"
#include "ArtisticTextShape.h"
#include <klocalizedstring.h>

ChangeTextFontCommand::ChangeTextFontCommand(ArtisticTextShape *shape, const QFont &font, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_newFont(font)
    , m_rangeStart(-1)
    , m_rangeCount(-1)
{
    Q_ASSERT(m_shape);
    setText(kundo2_i18n("Change font"));
}

ChangeTextFontCommand::ChangeTextFontCommand(ArtisticTextShape *shape, int from, int count, const QFont &font, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_newFont(font)
    , m_rangeStart(from)
    , m_rangeCount(count)
{
    Q_ASSERT(m_shape);
}

void ChangeTextFontCommand::redo()
{
    if (m_oldText.isEmpty()) {
        m_oldText = m_shape->text();
        if (m_rangeStart >= 0) {
            m_shape->setFont(m_rangeStart, m_rangeCount, m_newFont);
        } else {
            m_shape->setFont(m_newFont);
        }
        if (m_newText.isEmpty()) {
            m_newText = m_shape->text();
        }
    } else {
        m_shape->clear();
        Q_FOREACH (const ArtisticTextRange &range, m_newText) {
            m_shape->appendText(range);
        }
    }
}

void ChangeTextFontCommand::undo()
{
    m_shape->clear();
    Q_FOREACH (const ArtisticTextRange &range, m_oldText) {
        m_shape->appendText(range);
    }
}

