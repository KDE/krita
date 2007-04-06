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
#ifndef KOTEXTANCHOR_H
#define KOTEXTANCHOR_H

#include "KoInlineObject.h"

#include <kotext_export.h>

#include <QPointF>

class KoShape;

/**
 * This class is the object that is positioned in the text to be an anchor for a shape.
 * The idea is that when the text is relayouted and this inline character is moved, the shape
 * associated with it is repositioned based on a set of rules influenced by the data on this anchor.
 */
class KOTEXT_EXPORT KoTextAnchor : public KoInlineObject {
public:
    /// the vertical alignment options for the shape this anchor holds.
    enum AnchorVertical {
        TopOfFrame,
        TopOfParagraph,
        AboveCurrentLine,
        BelowCurrentLine,
        BottomOfParagraph,
        BottomOfFrame,
        VerticalOffset
    };
    /// the horizontal alignment options for the shape this anchor holds.
    enum AnchorHorizontal {
        Left,
        Right,
        Center,
        ClosestToBinding,
        FurtherFromBinding,
        HorizontalOffset
    };

    KoTextAnchor(KoShape *shape);
    ~KoTextAnchor();

    KoShape *shape() const;

    void setAlignment(AnchorHorizontal horizontal);
    void setAlignment(AnchorVertical vertical);
    AnchorVertical verticalAlignment() const;
    AnchorHorizontal horizontalAlignment() const;

    int positionInDocument() const;
    const QTextDocument *document() const;

    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format);
    virtual void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    virtual void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

    const QPointF &offset() const;

private:
    class Private;
    Private * const d;
};

#endif
