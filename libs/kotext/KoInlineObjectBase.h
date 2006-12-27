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
#ifndef KOINLINEOBJECTBASE_H
#define KOINLINEOBJECTBASE_H
#include <QHash>
#include <QTextDocument>
#include <QTextFormat>
#include <QTextInlineObject>

class KoInlineObjectBase {
public:
    KoInlineObjectBase() : m_id(-1) {}
    virtual ~KoInlineObjectBase() {}

    virtual void updatePosition(const QTextDocument &document, QTextInlineObject item,
            int posInDocument, const QTextFormat & format ) = 0;
    virtual void resize(const QTextDocument &document, QTextInlineObject item,
            int posInDocument, const QTextFormat & format ) = 0;
    virtual void paint (QPainter &painter, const QTextDocument &document, const QRectF &rect,
            QTextInlineObject object, int posInDocument, const QTextFormat &format) = 0;

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
private:
    int m_id;
};
#endif
