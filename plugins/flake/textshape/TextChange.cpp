/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "TextChange.h"

TextChange::TextChange()
    : m_formerPosition(0)
    , m_position(0)
    , m_previous(0)
    , m_next(0)
{
}

int TextChange::length() const
{
    return m_after.length() - m_before.length();
}

int TextChange::formerPosition() const
{
    return m_formerPosition;
}

int TextChange::position() const
{
    return m_position;
}

int TextChange::formerLength() const
{
    // TODO
    return -1;
}

void TextChange::setPosition(int pos)
{
    m_position = pos;
    m_formerPosition = pos;
}

void TextChange::setOldText(const QString &old)
{
    m_before = old;
}

void TextChange::setNewText(const QString &current)
{
    m_after = current;
}

void TextChange::setPrevious(TextChange *item)
{
    m_previous = item;
}

void TextChange::setNext(TextChange *item)
{
    m_next = item;
}

void TextChange::move(int length)
{
    m_position += length;
    if (m_next) {
        m_next->move(length);
    }
}

void TextChange::insertBefore(TextChange *node)
{
    move(node->length());
    node->setPrevious(previous());
    node->setNext(this);
    setPrevious(node);
    if (node->previous()) {
        node->previous()->setNext(node);
    }
}

void TextChange::insertAfter(TextChange *node)
{
    node->setPrevious(this);
    node->setNext(next());
    setNext(node);
    if (node->next()) {
        node->next()->setPrevious(node);
    }
}

void TextChange::merge(TextChange *other)
{
    // make sure the start of 'other' is within this change instance
    Q_ASSERT(other->position() >= position() && other->position() <= position() + length());

    /// this only does very simple merging for now.
    m_after.insert(other->position() - m_position, other->after());
}

