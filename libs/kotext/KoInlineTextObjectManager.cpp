/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KoInlineTextObjectManager.h"

#include <QTextCursor>
#include <QPainter>

KoInlineTextObjectManager::KoInlineTextObjectManager()
    : m_lastObjectId(0)
{
}

KoInlineObjectBase *KoInlineTextObjectManager::inlineTextObject(const QTextFormat &format) const {
    int id = format.intProperty(InlineInstanceId);
    if(id <= 0)
        return 0;
    return m_objects.value(id);
}

KoInlineObjectBase *KoInlineTextObjectManager::inlineTextObject(const QTextCursor &cursor) const {
    return inlineTextObject(cursor.charFormat());
}

void KoInlineTextObjectManager::insertInlineObject(QTextCursor &cursor, KoInlineObjectBase *object) {
    QTextCharFormat cf;
    cf.setObjectType(1001);
    cf.setProperty(InlineInstanceId, ++m_lastObjectId);
    cursor.insertText(QString(0xFFFC), cf);
    object->setId(m_lastObjectId);
    m_objects.insert(m_lastObjectId, object);
}
