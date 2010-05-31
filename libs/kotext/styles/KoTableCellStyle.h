/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#ifndef KOTABLECELLSTYLE_H
#define KOTABLECELLSTYLE_H

#include "KoText.h"
#include "kotext_export.h"

#include <QColor>

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QPainter>

struct Property;
class QTextTableCell;
class QRectF;
class QPainter;
class QPainterPath;
class KoStyleStack;
class KoGenStyle;
class KoGenStyles;
#include "KoXmlReaderForward.h"
class KoOdfLoadingContext;

/**
 * A container for all properties for the table cell style.
 * Each tablecell in the main text either is based on a table cell style, or its not. Where
 * it is based on a table cell style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoTableCellStyle.
 * @see KoStyleManager
 */
class KOTEXT_EXPORT KoTableCellStyle : public QObject
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
        CellBackgroundBrush,     ///< the cell background brush, as QTextFormat::BackgroundBrush is used by paragraphs
        MasterPageName,         ///< Optional name of the master-page
        InlineRdf               ///< Optional KoTextInlineRdf object
    };

    enum Side {
        Top = 0, ///< References the border at the top of the paragraph
        Left,    ///< References the border at the left side of the paragraph
        Bottom,  ///< References the border at the bottom of the paragraph
        Right    ///< References the border at the right side of the paragraph
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
    KoTableCellStyle(QObject *parent = 0);
    /// Creates a KoTableCellStyle with the given table cell format, and \a parent
    KoTableCellStyle(const QTextTableCellFormat &tableCellFormat, QObject *parent = 0);
    /// Destructor
    ~KoTableCellStyle();

    /// Creates a KoTableCellStyle that represents the formatting of \a block.
    static KoTableCellStyle *fromTableCell(const QTextTableCell &table, QObject *parent = 0);

    /// creates a clone of this style with the specified parent
    KoTableCellStyle *clone(QObject *parent = 0);

    /// returns if the borderstyle needs to be specially drawn
    bool isDrawn(BorderStyle style) const;

    /// draws a horizontal wave line
    void drawHorizontalWave(BorderStyle style, QPainter &painter, qreal x, qreal w, qreal t) const;

    /// draws a vertical wave line
    void drawVerticalWave(BorderStyle style, QPainter &painter, qreal y, qreal h, qreal t) const;

    /**
     * Get the bounding rectange \boundingRect adjusted according to the paddings and margins
     * of this border data. The inverse of this function is boundingRect().
     *
     * \sa boundingRect()
     *
     * @param the bounding rectangle.
     * @return the adjusted rectangle.
     */
    QRectF contentRect(const QRectF &boundingRect) const;

    /**
     * Get the bounding rect given a content rect, this is the inverse of contentRect().
     *
     * \sa contentRect()
     *
     * @param contentRect the content rectange.
     * @return the bounding rectange.
     */
    QRectF boundingRect(const QRectF &contentRect) const;

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
     * Paint the background.
     *
     * @painter the painter to draw with.
     * @bounds the bounding rectangle to draw.
     */
    void paintBackground(QPainter &painter, const QRectF &bounds) const;

    /**
     * Paint the borders.
     *
     * @painter the painter to draw with.
     * @bounds the bounding rectangle to draw.
     */
    void paintBorders(QPainter &painter, const QRectF &bounds) const;

    /**
     * Paint the top border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @w the width.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawTopHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QPainterPath *blanks = 0) const;

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
    void drawSharedHorizontalBorder(QPainter &painter, const KoTableCellStyle &styleBelow,  qreal x, qreal y, qreal w, QPainterPath *blanks = 0) const;

    /**
     * Paint the bottom border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @w the width.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawBottomHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QPainterPath *blanks = 0) const;

    /**
     * Paint the leftmost border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @h the height.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawLeftmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QPainterPath *blanks = 0) const;

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
    void drawSharedVerticalBorder(QPainter &painter, const KoTableCellStyle &styleRight,  qreal x, qreal y, qreal h, QPainterPath *blanks = 0) const;

    /**
     * Paint the rightmost border.
     *
     * @painter the painter to draw with.
     * @x the x position.
     * @y the y position.
     * @h the height.
     * @blanks a painterpath where blank borders should be added to.
     */
    void drawRightmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QPainterPath *blanks = 0) const;

    void setBackground(const QBrush &brush);
    /// See similar named method on QTextBlockFormat
    QBrush background() const;
    /// See similar named method on QTextBlockFormat
    void clearBackground();

    void setLeftPadding(qreal padding);
    void setTopPadding(qreal padding);
    void setRightPadding(qreal padding);
    void setBottomPadding(qreal padding);
    void setPadding(qreal padding);

    /// set the parent style this one inherits its unset properties from.
    void setParentStyle(KoTableCellStyle *parent);

    /// return the parent style
    KoTableCellStyle *parentStyle() const;

    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /// return the optional name of the master-page or a QString() if this paragraph isn't attached to a master-page.
    QString masterPageName() const;
    /// Set the name of the master-page.
    void setMasterPageName(const QString &name);


    /// copy all the properties from the other style to this style, effectively duplicating it.
    void copyProperties(const KoTableCellStyle *style);

    /**
     * Apply this style to a blockFormat by copying all properties from this, and parent
     * styles to the target block format.  Note that the character format will not be applied
     * using this method, use the other applyStyle() method for that.
     */
    void applyStyle(QTextTableCellFormat &format) const;

    void remove(int key);

    /// Compare the paragraph, character and list properties of this style with the other
    bool operator==(const KoTableCellStyle &other) const;

    void removeDuplicates(const KoTableCellStyle &other);

    /**
     * Load the style form the element
     *
     * @param context the odf loading context
     * @param element the element containing the
     */
    void loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context);

    void saveOdf(KoGenStyle &style);

    /**
     * Returns true if this paragraph style has the property set.
     * Note that this method does not delegate to the parent style.
     * @param key the key as found in the Property enum
     */
    bool hasProperty(int key) const;

    /**
     * Set a property with key to a certain value, overriding the value from the parent style.
     * If the value set is equal to the value of the parent style, the key will be removed instead.
     * @param key the Property to set.
     * @param value the new value to set on this style.
     * @see hasProperty(), value()
     */
    void setProperty(int key, const QVariant &value);
    /**
     * Return the value of key as represented on this style, taking into account parent styles.
     * You should consider using the direct accessors for individual properties instead.
     * @param key the Property to request.
     * @returns a QVariant which holds the property value.
     */
    QVariant value(int key) const;

signals:
    void nameChanged(const QString &newName);

private:
    /**
     * Load the style from the \a KoStyleStack style stack using the
     * OpenDocument format.
     */
    void loadOdfProperties(KoStyleStack &styleStack);
    qreal propertyDouble(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QColor propertyColor(int key) const;
    BorderStyle oasisBorderStyle(const QString &borderstyle);
    QString odfBorderStyleString(const BorderStyle borderstyle);

    class Private;
    Private * const d;
};

#endif
