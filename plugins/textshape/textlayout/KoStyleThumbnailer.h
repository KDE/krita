/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009-2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "kritatextlayout_export.h"

#include <QSize>

class KoCharacterStyle;
class KoParagraphStyle;

class QImage;

/**
 * Helper class to create (and cache) thumbnails of styles
 */
class KRITATEXTLAYOUT_EXPORT KoStyleThumbnailer
{
public:
    enum KoStyleThumbnailerFlag {
        NoFlags = 0,
        CenterAlignThumbnail = 1, ///< Vertically Center Align the layout of the thumbnail
                                  ///     i.e the layout is done at the center of the area
        UseStyleNameText = 2, ///< Use the style name as the text that is layouted inside the thumbnail
        ScaleThumbnailFont = 4 ///< If set, then when the layout size is more than the size available
                               ///  the font size is scaled down to fit the space availiable
    };
    Q_DECLARE_FLAGS(KoStyleThumbnailerFlags, KoStyleThumbnailerFlag)

    /**
     * Create a new style thumbnailer.
     */
    explicit KoStyleThumbnailer();

    /**
     * Destructor.
     */
    virtual ~KoStyleThumbnailer();

    /**
     * @returns a thumbnail representing the @param style, constrained into the @param size.
     * If there is no specified @param size, the thunbnail is the size specified with @fn setThumbnailSize or 250*48 pt if no size was provided.
     * If the given @param size is too small, the font size will be decreased, so the thumbnail fits.
     * The real font size is indicated in this case.
     * If @param recreateThumbnail is true, do not return the cached thumbnail if it exist, but recreate a new one.
     * The created thumbnail is cached.
     */
    QImage thumbnail(KoParagraphStyle *style,
                     const QSize &size = QSize(), bool recreateThumbnail = false,
                     KoStyleThumbnailerFlags flags =
                          KoStyleThumbnailerFlags(CenterAlignThumbnail | UseStyleNameText | ScaleThumbnailFont));

    /**
     * @returns a thumbnail representing the @param characterStyle applied on the given @param paragraphStyle, constrained into the @param size.
     * If there is no specified @param size, the thunbnail is the size specified with @fn setThumbnailSize or 250*48 pt if no size was provided.
     * If the given @param size is too small, the font size will be decreased, so the thumbnail fits.
     * The real font size is indicated in this case.
     * If @param recreateThumbnail is true, do not return the cached thumbnail if it exist, but recreate a new one.
     * The created thumbnail is cached.
     */
    QImage thumbnail(KoCharacterStyle *characterStyle, KoParagraphStyle *paragraphStyle = 0,
                     const QSize &size = QSize(), bool recreateThumbnail = false,
                     KoStyleThumbnailerFlags flags =
                          KoStyleThumbnailerFlags(CenterAlignThumbnail | UseStyleNameText | ScaleThumbnailFont));

    /**
     * Sets the size of the thumbnails returned by the @fn thumbnail with no size arguments.
     */
    void setThumbnailSize(const QSize &size);

    /**
     * Sets the text that will be layouted.
     * @param text The text that will be layouted inside the thumbnail
     *If the UseStyleNameText flag is set then this text will not be used
     */
    void setText(const QString &text);

    /**
     * remove all occurrences of the style from the cache
     */
    void removeFromCache(KoParagraphStyle *style);

    /**
     * remove all occurrences of the style from the cache
     */
    void removeFromCache(KoCharacterStyle *style);

private:
    void layoutThumbnail(const QSize &size, QImage *im, KoStyleThumbnailerFlags flags);
    void removeFromCache(const QString &expr);

    class Private;
    Private* const d;
};

#endif
