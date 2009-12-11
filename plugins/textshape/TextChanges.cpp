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

#include "TextChanges.h"
#include "TextChange.h"


TextChanges::TextChanges()
        : m_root(0)
{
}

TextChanges::~TextChanges()
{
    TextChange *change = m_root;
    while (change) {
        TextChange *prev = change;
        change = change->next();
        delete prev;
    }
    m_root = 0;
}

void TextChanges::inserted(int position, const QString &text)
{
    changed(position, QString(), text);
}

void TextChanges::changed(int position, const QString &former, const QString &latter)
{
    TextChange *change = new TextChange();
    change->setPosition(position);
    change->setNewText(latter);
    change->setOldText(former);
    if (m_root == 0) {
        m_root = change;
        return;
    }

    TextChange *cursor = m_root;
    while (cursor->next()) {
        if (cursor->position() + cursor->length() >= position) break;
        cursor = cursor->next();
    }
    Q_ASSERT(cursor);
    if (cursor->position() > position) { // insert new one before.
        cursor->insertBefore(change);
        if (cursor == m_root)
            m_root = change;
    } else if (position >= cursor->position() && position <= cursor->position() + cursor->length()) {//merge
        cursor->merge(change);
        delete change;
    } else  { // insert new one after.
        cursor->insertAfter(change);
        if (change->next())
            change->next()->move(change->length());
    }
}

bool TextChanges::hasText(int position, int length) const
{
    Q_UNUSED(position);
    Q_UNUSED(length);
    return false;
}

QMap<int, const TextChange*> TextChanges::changes() const
{
    QMap<int, const TextChange*> result;
    return result;
}
