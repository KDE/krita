/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
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

#ifndef KOTEXTMETA_H
#define KOTEXTMETA_H

#include "KoInlineObject.h"
#include "kritatext_export.h"

/**
 * Used to indicate an ODF text:meta container. This is very similar to a KoBookmark
 * in that a specific start-end is marked.
 */
class KRITATEXT_EXPORT KoTextMeta : public KoInlineObject
{
    Q_OBJECT
public:
    enum BookmarkType {
        StartBookmark,      ///< start position
        EndBookmark         ///< end position
    };

    /**
     * Constructor
     * @param name the name for this bookmark
     * @param document the text document where this bookmark is located
     */
    explicit KoTextMeta(const QTextDocument *document);

    virtual ~KoTextMeta();

    /// reimplemented from super
    void saveOdf(KoShapeSavingContext &context);
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /// reimplemented from super
    virtual void updatePosition(const QTextDocument *document,
                                int posInDocument, const QTextCharFormat &format);
    /// reimplemented from super
    virtual void resize(const QTextDocument *document, QTextInlineObject &object,
                        int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from super
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                       const QRectF &rect, const QTextInlineObject &object, int posInDocument, const QTextCharFormat &format);

    void setType(BookmarkType type);

    /// @return the current type of this bookmark
    BookmarkType type() const;

    void setEndBookmark(KoTextMeta *bookmark);

    /// @return the end bookmark if the type is StartBookmark
    KoTextMeta* endBookmark() const;

    /// @return the exact cursor position of this bookmark in document
    int position() const;

private:
    class Private; // TODO share the private with super
    Private *const d;
};

#endif

