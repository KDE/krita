/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef KODELETECHANGEMARKER_H
#define KODELETECHANGEMARKER_H

#include "kotext_export.h"

#include <KoInlineObject.h>

class KoXmlElement;
class KoChangeTracker;

class KOTEXT_EXPORT KoDeleteChangeMarker : public KoInlineObject
{
public:

    KoDeleteChangeMarker(KoChangeTracker *changeTracker);

    virtual ~KoDeleteChangeMarker();

    /**
    * Store the deleted text.
    * @param text the deleted text
    */
//    void setText(const QString &text);

    /// return the deleted text
//    QString text() const;

    /**
    * Set the changeTracker id for this change.
    * @param id the new id
    */
    void setChangeId(int id);

    /// return the change id
    int changeId() const;

    bool loadOdf(const KoXmlElement &element);
    ///reimplemented
    virtual void saveOdf(KoShapeSavingContext &context);

protected:

    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

    virtual void resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);

    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

private:

    class Private;
    Private * const d;
};

#endif // KODELETECHANGEMARKER_H
