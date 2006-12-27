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
#ifndef KOINLINEOBJECTMANAGER_H
#define KOINLINEOBJECTMANAGER_H

// KOffice libs
#include <KoInlineObjectBase.h>
#include <koffice_export.h>

// Qt + kde
#include <QHash>
#include <QTextDocument>
#include <QTextFormat>
#include <QTextInlineObject>

class KOTEXT_EXPORT KoInlineTextObjectManager {
// TODO, when to delete the inlineObject s
public:
    KoInlineTextObjectManager();

    KoInlineObjectBase *inlineTextObject(const QTextFormat &format) const;
    KoInlineObjectBase *inlineTextObject(const QTextCursor &cursor) const;

    void insertInlineObject(QTextCursor &cursor, KoInlineObjectBase *object);

private:
    enum Properties {
        InlineInstanceId = QTextFormat::UserProperty+7001
    };

    QHash<int, KoInlineObjectBase*> m_objects;
    int m_lastObjectId;
};

#endif
