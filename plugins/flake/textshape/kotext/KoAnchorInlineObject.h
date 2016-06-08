/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 * Copyright (C) 2013 C. Boemann <cbo@boemann.dk>
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
#ifndef KOANCHORINLINEOBJECT_H
#define KOANCHORINLINEOBJECT_H

#include "KoInlineObject.h"

#include "KoShapeAnchor.h"

#include "kritatext_export.h"

class KoAnchorInlineObjectPrivate;

/**
 * This class connects KoShapeAnchor to an inline character in the text document.
 *
 * This class is used when the shape anchor is of type: as-char
 *
 * It has to be registered to the inlineobjectmanager and thus forms the connection between the text
 * and the KoShapeAnchor and by extension the so called 'anchored-shape' (any kind of shape)
 *
 * The KoAnchorInlineObject is placed as a character in text. As such it will move and be
 * editable like any other character, including deletion.
 *
 * Since this is a real character it will be positioned by the textlayout engine and anything that
 * will change the position of the text will thus also change the KoAnchorInlineObject character.
 *
 * The anchored-shape can be repositioned on the canvas if the text is relayouted (for example after
 * editing the text. This is dependent on how the text layout is implemented.
 *
 * Steps to use a KoAnchorInlineObject are; <ol>
 * <li> Create KoShapeAnchor *anchor = new KoShapeAnchor(shape);
 * <li> Use anchor->loadOdf() to load additional attributes like the "text:anchor-type"
 * <li> if type is as-char create KoAnchorInlineObject *anchorObj = new KoAnchorInlineObject(anchor);
 */
class KRITATEXT_EXPORT KoAnchorInlineObject : public KoInlineObject, public KoShapeAnchor::TextLocation
{
    Q_OBJECT
public:
    /**
     * Constructor for an as-char anchor.
     * @param parent the shapeanchor.
     */
    explicit  KoAnchorInlineObject(KoShapeAnchor *parent);
    virtual ~KoAnchorInlineObject();

    /// returns the parent anchor
    KoShapeAnchor *anchor() const;

    /// returns the cursor position in the document where this anchor is positioned.
    int position() const;

    /// returns the document that this anchor is associated with.
    const QTextDocument *document() const;

    /// reimplemented from KoInlineObject
    virtual void updatePosition(const QTextDocument *document,
                                int posInDocument, const QTextCharFormat &format);
    /// reimplemented from KoInlineObject
    virtual void resize(const QTextDocument *document, QTextInlineObject &object,
                        int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from KoInlineObject
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                       const QRectF &rect, const QTextInlineObject &object, int posInDocument, const QTextCharFormat &format);

    qreal inlineObjectAscent() const;

    qreal inlineObjectDescent() const;

    /// reimplemented from KoInlineObject - should not do anything
    bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &);

    /// reimplemented from KoInlineObject
    void saveOdf(KoShapeSavingContext &context);


private:
    Q_DECLARE_PRIVATE(KoAnchorInlineObject)
};

#endif
