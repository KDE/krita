/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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

#include "KoTextRange.h"
#include "kritatext_export.h"

class KoBookmarkManager;

/**
 * A document can store a set of cursor positions/selected cursor locations which can be
 * retrieved again later to go to those locations from any location in the document.
 * The bookmark location will be automatically updated if user alters the text in the document.
 * A bookmark is identified by it's name, and all bookmarks are managed by KoBookmarkManager. A
 * bookmark can be retrieved from the bookmark manager by using name as identifier.
 * @see KoBookmarkManager
 */
class KRITATEXT_EXPORT KoBookmark : public KoTextRange
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * By default a bookmark has the SinglePosition type and an empty name.
     * The name is set when the book is inserted into the bookmark manager.
     *
     * @param document the text document where this bookmark is located
     */
    explicit KoBookmark(const QTextCursor &);

    virtual ~KoBookmark();

    /// reimplemented from super
    void saveOdf(KoShapeSavingContext &context, int position, TagType tagType) const;

    /**
     * Set the new name for this bookmark
     * @param name the new name of the bookmark
     */
    void setName(const QString &name);

    /// @return the name of this bookmark
    QString name() const;

    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * This is called to allow Cut and Paste of bookmarks. This
     * method gives a correct, unique, name
     */
    static QString createUniqueBookmarkName(const KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker);
private:

    class Private;
    Private *const d;
};

#endif

