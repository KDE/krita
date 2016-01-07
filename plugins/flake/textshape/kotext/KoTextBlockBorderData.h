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
#ifndef KOTEXTBLOCKBORDERDATA_H
#define KOTEXTBLOCKBORDERDATA_H

#include "styles/KoParagraphStyle.h"

#include "kritatext_export.h"

class QRectF;
class QPainter;

/**
 * This class holds data for paragraph-borders.
 * All the information needed to paint the borders, but also to calculate the insets that the borders
 * cause on text-layout is stored in here.
 * An instance of this class is owned by the KoTextBlockData and this class is being refcounted
 * to allow multiple paragraphs to share one border.
 *
 */
class KRITATEXT_EXPORT KoTextBlockBorderData
{
public:
    /// Enum used to differentiate between the 4 types of borders this class maintains
    enum Side {
        Top = 0, ///< References the border at the top of the paragraph
        Left,   ///< References the border at the left side of the paragraph
        Bottom, ///< References the border at the bottom of the paragraph
        Right   ///< References the border at the right side of the paragraph
    };
    /**
     * Constructor for the border data.
     * Will create a border-set with the param rectangle but all the borders will
     * be turned off.
     * @param paragRect the rectangle that will be used to paint the border inside of.
     * @see setEdge() to set the actual border properties.
     */
    explicit KoTextBlockBorderData(const QRectF &paragRect);

    /**
     * Copy constructor for the border data.
     * @param other the original object which will be duplicated.
     */
    explicit KoTextBlockBorderData(const KoTextBlockBorderData &other);

    ~KoTextBlockBorderData();

    /**
     * Increments the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool ref();
    /**
     * Decrements the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool deref();
    /// return the usage count
    int useCount() const;

    /**
     * Set the properties of an edge based on a paragraph-format.
     * @param side defines which edge this is for.
     * @param bf the format of the paragraph See QTextBlock.blockFormat()
     * @param style the border style for this side.
     * @param width the thickness of the border-line.
     * @param color the property for the color of the border-line(s).
     * @param space the amount of spacing between the outer border and the inner border in case of style being double
     * @param innerWidth the thickness of the inner border-line in case of style being double
     */
    void setEdge(Side side, const QTextBlockFormat &bf, KoParagraphStyle::Property style,
                 KoParagraphStyle::Property width, KoParagraphStyle::Property color,
                 KoParagraphStyle::Property space, KoParagraphStyle::Property innerWidth);

    /**
     * Set if this border should possibly be merged with the next.
     */
    void setMergeWithNext(bool merge);

    /**
     * @return true if there has been at least one border set.
     */
    bool hasBorders() const;

    /**
     * Find the inset that a border causes for a specific side.
     * @see applyInsets()
     */
    qreal inset(Side side) const;

    /// returns true if the borders of param border are the same as this one.
    bool operator==(const KoTextBlockBorderData &border) const;
    bool equals(const KoTextBlockBorderData &border) const;

    /**
     * Paint the borders.
     */
    void paint(QPainter &painter, const QRectF &clip) const;

private:
    class Private;
    Private * const d;
};

#endif
