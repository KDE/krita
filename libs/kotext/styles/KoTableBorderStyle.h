/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#ifndef KOTABLEBORDERSTYLE_H
#define KOTABLEBORDERSTYLE_H

#include "KoText.h"
#include "kotext_export.h"

class KoTableBorderStylePrivate;

class KOTEXT_EXPORT KoTableBorderStyle : public QObject
{
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextTableCellFormat::UserProperty + 7001,
        TopBorderOuterPen, ///< the top border pen
        TopBorderSpacing,          ///< the top border spacing between inner and outer border
        TopBorderInnerPen,       ///< the top border inner pen
        TopBorderStyle,       ///< the top border borderstyle
        LeftBorderOuterPen,      ///< the left border outer pen
        LeftBorderSpacing,         ///< the left border spacing between inner and outer border
        LeftBorderInnerPen,      ///< the left border inner pen
        LeftBorderStyle,       ///< the left border borderstyle
        BottomBorderOuterPen,    ///< the bottom border outer pen
        BottomBorderSpacing,       ///< the bottom border spacing between inner and outer border
        BottomBorderInnerPen,    ///< the bottom border inner pen
        BottomBorderStyle,       ///< the bottom border borderstyle
        RightBorderOuterPen,     ///< the right border outer pen
        RightBorderSpacing,        ///< the right border spacing between inner and outer border
        RightBorderInnerPen,     ///< the right border inner pen
        RightBorderStyle,       ///< the right border borderstyle
        TopLeftToBottomRightBorderOuterPen, ///< the top left to bottom right diagonal border pen
        TopLeftToBottomRightBorderSpacing,  ///< the top left to bottom right diagonal spacing
        TopLeftToBottomRightBorderInnerPen, ///< the top left to bottom right diagonal inner pen
        TopLeftToBottomRightBorderStyle,    ///< the top left to bottom right borderstyle
        BottomLeftToTopRightBorderOuterPen, ///< the bottom letf to top right diagonal border pen
        BottomLeftToTopRightBorderSpacing,  ///< the bottom letf to top right diagonal spacing
        BottomLeftToTopRightBorderInnerPen, ///< the bottom letf to top right diagonal inner pen
        BottomLeftToTopRightBorderStyle,    ///< the bottom letf to top right borderstyle
        CellBackgroundBrush,     ///< the cell background brush, as QTextFormat::BackgroundBrush is used by paragraphs
        VerticalAlignment,     ///< the vertical alignment oinside the cell
        MasterPageName,         ///< Optional name of the master-page
        InlineRdf               ///< Optional KoTextInlineRdf object
    };

    enum Side {
        Top = 0, ///< References the border at the top of the cell
        Left,    ///< References the border at the left side of the cell
        Bottom,  ///< References the border at the bottom of the cell
        Right,   ///< References the border at the right side of the paragraph
        TopLeftToBottomRight, ///< References the border from top, left corner to bottom, right corner of cell
        BottomLeftToTopRight  ///< References the border from bottom, left corner to top, right corner of cell
    };

    /// Enum used to differentiate between the 3 types of border styles
    enum BorderStyle {
        BorderNone = 0, ///< No line border
        BorderSolid,    ///< Solid line border
        BorderDotted,    ///< Dotted single border
        BorderDashDot,    ///< Dot Dashsingle border
        BorderDashDotDot,    ///< Dot Dot Dash single border
        BorderDashed,    ///< Dashed single border
        BorderDashedLong,    ///< Dashed single border with long spaces
        BorderDouble,    ///< Double lined border
        BorderTriple,    ///< Triple lined border
        BorderSlash,    ///< slash border
        BorderWave,    ///< wave border
        BorderDoubleWave    ///< double wave border
    };


    /// Constructor
    KoTableBorderStyle(QObject *parent = 0);
    /// Creates a KoTableBorderStyle with the given table cell format, and \a parent
    KoTableBorderStyle(const QTextTableCellFormat &tableCellFormat, QObject *parent = 0);
    /// Destructor
    ~KoTableBorderStyle();

    /// returns if the borderstyle needs to be specially drawn
    bool isDrawn(BorderStyle style) const;

    /// draws a horizontal wave line
    void drawHorizontalWave(BorderStyle style, QPainter &painter, qreal x, qreal w, qreal t) const;

    /// draws a vertical wave line
    void drawVerticalWave(BorderStyle style, QPainter &painter, qreal y, qreal h, qreal t) const;

    /**
     * Set the properties of an edge.
     *
     * @param side defines which edge this is for.
     * @param style the border style for this side.
     * @param totalWidth the thickness of the border. Sum of outerwidth, spacing and innerwidth for double borders
     * @param color the color of the border line(s).
     */
    void setEdge(Side side, BorderStyle style, qreal totalWidth, QColor color);

    /**
     * Set the properties of an double border.
     * Note: you need to set the edge first or that would overwrite these values.
     *
     * The values will not be set if the border doesn't have a double style
     *
     * @param side defines which edge this is for.
     * @param space the amount of spacing between the outer border and the inner border in case of style being double
     * @param innerWidth the thickness of the inner border line in case of style being double
     */
    void setEdgeDoubleBorderValues(Side side, qreal innerWidth, qreal space);

    /**
     * Check if the border data has any borders.
     *
     * @return true if there has been at least one border set.
     */
    bool hasBorders() const;

    /**
     * Paint the borders.
     *
     * @painter the painter to draw with.
     * @bounds the bounding rectangle to draw.
     * @blanks a painterpath where blank borders should be added to.
     */
    void paintBorders(QPainter &painter, const QRectF &bounds, QVector<QLineF> *blanks) const;

    /**
     * Paint the diagonal borders.
     *
     * @painter the painter to draw with.
     * @bounds the bounding rectangle to draw.
     */
    void paintDiagonalBorders(QPainter &painter, const QRectF &bounds) const;

    /**
     * Paint the top border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @w the width.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawTopHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the border that is shared.
     * It only draws the thickest and it always draws it below the y position.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @w the width.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawSharedHorizontalBorder(QPainter &painter, const KoTableBorderStyle &styleBelow,  qreal x, qreal y, qreal w, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the bottom border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @w the width.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawBottomHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the leftmost border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @h the height.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawLeftmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the border that is shared.
     * It only draws the thickest and it always draws it below the y position.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @h the height.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawSharedVerticalBorder(QPainter &painter, const KoTableBorderStyle &styleRight,  qreal x, qreal y, qreal h, QVector<QLineF> *blanks = 0) const;

    /**
     * Paint the rightmost border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @h the height.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawRightmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *blanks = 0) const;

    qreal leftBorderWidth() const;
    qreal rightBorderWidth() const;
    qreal topBorderWidth() const;
    qreal bottomBorderWidth() const;

protected:
    KoTableBorderStyle(KoTableBorderStylePrivate &dd, const QTextTableCellFormat &format, QObject *parent);
    KoTableBorderStyle(KoTableBorderStylePrivate &dd, QObject *parent);
    KoTableBorderStylePrivate *d_ptr;

private:
    void init(const QTextTableCellFormat &format);

    Q_DECLARE_PRIVATE(KoTableBorderStyle)
};

#endif // KOTABLEBORDERSTYLE_H

