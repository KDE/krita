/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KOBOOKMARK_H
#define KOBOOKMARK_H

#include "KoInlineObject.h"
#include "kotext_export.h"

class KoShape;
class QTextDocument;
class KoShapeSavingContext;
class KoShapeLoadingContext;

/**
 * A document can store a set of cursor positions/selected cursor locations which can be
 * retrieved again later to go to those locations from any location in the document.
 * Since a bookmark is basically an inline object, the bookmark location will be automatically
 * updated if user alters the text in the document. A bookmark is identified by it's name,
 * and all bookmarks are managed by KoBookmarkManager. A bookmark can be retrieved from the
 * bookmark manager by using name as identifier.
 * @see KoBookmarkManager
 * If a text selection is bookmarked, we need two bookmark objects to mark the selection.
 * One is for start selection and the other one is for end selection.
 * A bookmark should know the specific text shape where it is located since a document may
 * contain more than one text shapes. This shape can be retrieved using KoBookmark::shape()
 */
class KOTEXT_EXPORT KoBookmark : public KoInlineObject
{
public:
    /// This enum determines the type of bookmark
    enum BookmarkType {
        SinglePosition,     ///< single cursor position
        StartBookmark,      ///< start position of a bookmarked selection
        EndBookmark         ///< end position of a bookmarked selection
    };

    /**
     * Constructor
     * @param name the name for this bookmark
     * @param document the text document where this bookmark is located
     */
    KoBookmark(const QString &name, const QTextDocument *document);

    virtual ~KoBookmark();

    /// reimplemented from super
    void saveOdf(KoShapeSavingContext &context);
    /// reimplemented from super
    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object,
                                int posInDocument, const QTextCharFormat &format);
    /// reimplemented from super
    virtual void resize(const QTextDocument *document, QTextInlineObject object,
                        int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from super
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                       const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

    /**
     * Set the new name for this bookmark
     * If the bookmark type is StartBookmark, the end bookmark's name will also be set
     * @param name the new name of the bookmark
     */
    void setName(const QString &name);

    /// @return the name of this bookmark
    QString name() const;

    /**
     * Set the type for the bookmark object
     * @param type the new type of the bookmark object
     * @see BookmarkType
     */
    void setType(BookmarkType type);

    /// @return the current type of this bookmark
    BookmarkType type();

    /**
     * Set the end bookmark of this bookmark. This bookmark should be a StartBookmark.
     * @param bookmark the bookmark object which will be set as end bookmark
     */
    void setEndBookmark(KoBookmark *bookmark);

    /// @return the end bookmark if the type is StartBookmark
    KoBookmark *endBookmark();

    /// @return the KoShape where this bookmark is located
    KoShape *shape();

    /// @return the exact cursor position of this bookmark in document
    int position();

    /// @return true if this bookmark has selection (type is StartBookmark of EndBookmark)
    bool hasSelection();

    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

private:
    class Private;
    Private *const d;
};

#endif

