/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009-2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef KOSTYLETHUMBNAILER_H
#define KOSTYLETHUMBNAILER_H

#include <QPixmap>

class QSize;
class QTextDocument;
class KoCharacterStyle;
class KoParagraphStyle;

/**
 * Helper class to create (and cache) thumbnails of styles
 */
class KoStyleThumbnailer
{
public:
    /**
     * Create a new style thumbnailer.
     */
    explicit KoStyleThumbnailer();

    /**
     * Destructor.
     */
    virtual ~KoStyleThumbnailer();

    /**
     * Return a thumbnail representing the style
     * The thunbnail is the size specified with @fn setThumbnailSize or 250*48 pt if no size was provided.
     * If @param dirtyCache is true, do not return the cached thumbnail if it exist, but recreate a new one.
     * The created thumbnail is cached.
     */
    QPixmap thumbnail(KoParagraphStyle *style, bool dirtyCache = false);

    /**
     * @returns a thumbnail representing the @param style, constrained into the @param size.
     * If the given @param size is too small, the font size will be decreased, so the thumbnail fits.
     * The real font size is indicated in this case.
     * If @param dirtyCache is true, do not return the cached thumbnail if it exist, but recreate a new one.
     * The created thumbnail is cached.
     */
    QPixmap thumbnail(KoParagraphStyle *style, QSize size, bool dirtyCache = false);

    /**
     * Return a thumbnail representing the style
     * The thunbnail is the size specified with @fn setThumbnailSize or 250*48 pt if no size was provided.
     * If @param dirtyCache is true, do not return the cached thumbnail if it exist, but recreate a new one.
     * The created thumbnail is cached.
     */
    QPixmap thumbnail(KoCharacterStyle *style, bool dirtyCache = false);

    /**
     * @returns a thumbnail representing the @param style, constrained into the @param size.
     * If the given @param size is too small, the font size will be decreased, so the thumbnail fits.
     * The real font size is indicated in this case.
     * If @param dirtyCache is true, do not return the cached thumbnail if it exist, but recreate a new one.
     * The created thumbnail is cached.
     */
    QPixmap thumbnail(KoCharacterStyle *style, QSize size, bool dirtyCache = false);

    /**
     * Sets the size of the thumbnails returned by the @fn thumbnail with no size arguments.
     */
    void setThumbnailSize(QSize size);

private:
    void layoutThumbnail(QSize size, QPixmap &pm);

    class Private;
    Private* const d;
};

#endif
