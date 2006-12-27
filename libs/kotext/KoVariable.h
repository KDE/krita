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

// KOffice libs
#include <KoInlineObjectBase.h>
#include <koffice_export.h>

// Qt + kde
#include <QHash>
#include <QTextDocument>
#include <QTextInlineObject>

class KOTEXT_EXPORT KoVariable : public KoInlineObjectBase {
public:
    KoVariable(const QString &value = QString() );
    ~KoVariable() {}

    void setValue(const QString &value);

private:
    void updatePosition(const QTextDocument &document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format);
    void resize(const QTextDocument &document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument &document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

private:
    QString m_value;
    bool m_modified;
};
