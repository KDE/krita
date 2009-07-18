/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#ifndef KOTABLECELLBORDERDATA_H
#define KOTABLECELLBORDERDATA_H

#include "styles/KoParagraphStyle.h"
#include <KoColor.h>

#include <QPen>
#include <QRectF>
#include <QPainter>
#include <QTextTableCellFormat>
#include "kotext_export.h"

enum KoTableCellProperty{
    TopBorderOuterWidth = QTextFormat::UserProperty + 2000, ///< the top border outer width
    TopBorderSpacing,          ///< the top border spacing between inner and outer border
    TopBorderInnerWidth,       ///< the top border inner width
    LeftBorderOuterWidth,      ///< the left border outer width
    LeftBorderSpacing,         ///< the left border spacing between inner and outer border
    LeftBorderInnerWidth,      ///< the left border inner width
    BottomBorderOuterWidth,    ///< the bottom border outer width
    BottomBorderSpacing,       ///< the bottom border spacing between inner and outer border
    BottomBorderInnerWidth,    ///< the bottom border inner width
    RightBorderOuterWidth,     ///< the right border outer width
    RightBorderSpacing,        ///< the right border spacing between inner and outer border
    RightBorderInnerWidth,     ///< the right border inner width
};

/**
 * This class holds data for paragraph-borders.
 * All the information needed to paint the borders, but also to calculate the insets that the borders
 * cause on text-layout is stored in here.
 * An instance of this class is owned by the KoTextBlockData and this class is being refcounted
 * to allow multiple paragraphs to share one border.
 *
 */
class KOTEXT_EXPORT TableCellBorderData
{
public:
    /// Enum used to differentiate between the 4 types of borders this class maintains
    enum Side {
        Top = 0, ///< References the border at the top of the paragraph
        Left,    ///< References the border at the left side of the paragraph
        Bottom,  ///< References the border at the bottom of the paragraph
        Right    ///< References the border at the right side of the paragraph
    };

    /// Enum used to differentiate between the 3 types of border styles
    enum Style {
        None = 0, ///< No line border
        Solid,    ///< Solid line border
        Double    ///< Double lined border
    };

    /**
     * Constructor for the border data.
     * Will create a border-set but all the borders will be turned off.
     * @see setEdge() to set the actual border properties.
     */
    TableCellBorderData();

    /**
     * Copy constructor for the border data.
     * @param other the original object which will be duplicated.
     */
    explicit TableCellBorderData(const TableCellBorderData &other);

    ~TableCellBorderData();

    /// refcounting
    void addUser();
    /// refcounting
    int removeUser();
    /// refcounting
    int useCount() const;

    /**
     * Store all properties into a QTextTableCellFormat structure.
     *
     * @param format the QTextTableCellFormat.
     */
    void save(QTextTableCellFormat &format);

    /**
     * Restore all properties from a QTextTableCellFormat structure.
     *
     * @param format the QTextTableCellFormat:
     */
    void load(const QTextTableCellFormat &format);


    /**
     * Get the bouding rectange \boundingRect adjusted according to the paddings and margins
     * of this border data.
     *
     * @return the adjusted rectangle.
     */
    QRectF contentRect(const QRectF &boundingRect);

    /**
     * Set the properties of an edge based on a paragraph format.
     *
     * @param side defines which edge this is for.
     * @param style the border style for this side.
     * @param width the thickness of the border-line.
     * @param color the color of the border line(s).
     * @param space the amount of spacing between the outer border and the inner border in case of style being double
     * @param innerWidth the thickness of the inner border line in case of style being double
     */
    void setEdge(Side side, Style style, qreal width, KoColor color, qreal innerWidth);

    /**
     * Check if the border data has any borders.
     *
     * @return true if there has been at least one border set.
     */
    bool hasBorders() const;

    /// returns true if the borders of param border are the same as this one.
    bool operator==(const TableCellBorderData &border);
    bool equals(const TableCellBorderData &border);

    /**
     * Paint the borders.
     *
     * @painter the painter to draw with.
     * @bounds the bounding rectangle to draw.
     */
    void paint(QPainter &painter, const QRectF &bounds) const;

private:
    class Private;
    Private * const d;
};

#endif
