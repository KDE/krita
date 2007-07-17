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
#ifndef KOINLINENOTE_H
#define KOINLINENOTE_H

#include "KoInlineObject.h"
#include "kotext_export.h"

/** foot or end note */
class KOTEXT_EXPORT KoInlineNote : public KoInlineObject {
public:
    enum Type {
        Footnote,
        Endnote
        // Comment?
    };

    KoInlineNote(Type type);
    ~KoInlineNote();

    void setText(const QString &text);
    void setLabel(const QString &text);
    QString text() const;
    QString label() const;

    bool autoNumbering() const;
    void setAutoNumbering(bool on);

    Type type() const;

protected:
    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format);
    virtual void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    virtual void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

private:
    class Private;
    Private * const d;
};

#endif
