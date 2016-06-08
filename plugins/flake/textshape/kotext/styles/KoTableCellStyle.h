/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#include "kritatext_export.h"

#include <KoXmlReaderForward.h>

#include <KoBorder.h>
#include <KoShadowStyle.h>
#include <QColor>

#include <QObject>

struct Property;
class QTextTableCell;
class QRectF;
class KoStyleStack;
class KoGenStyle;
class KoParagraphStyle;
class KoShapeLoadingContext;
class KoShapeSavingContext;
class KoTableCellStylePrivate;
class QString;
class QVariant;

/**
 * A container for all properties for the table cell style.
 * Each tablecell in the main text either is based on a table cell style, or its not. Where
 * it is based on a table cell style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoTableCellStyle.
 * @see KoStyleManager
 */
class KRITATEXT_EXPORT KoTableCellStyle : public QObject
{
    Q_OBJECT
public:
    enum CellProtectionFlag {
        NoProtection,
        HiddenAndProtected,
        Protected,
        FormulaHidden,
        ProtectedAndFormulaHidden
    };

    enum CellTextDirection {
        Default = 0,
        LeftToRight,
        TopToBottom
    };

    enum RotationAlignment {
        RAlignNone,
        RAlignBottom,
        RAlignTop,
        RAlignCenter
    };

    enum Property {
        StyleId = QTextTableCellFormat::UserProperty + 7001,
        ShrinkToFit,                ///< Shrink the cell content to fit the size
        Wrap,                       ///< Wrap the text within the cell
        CellProtection,             ///< The cell protection when the table is protected
        PrintContent,               ///< Should the content of this cell be printed
        RepeatContent,              ///< Display the cell content as many times as possible
        DecimalPlaces,              ///< Count the maximum number of decimal places to display
        AlignFromType,              ///< Should the alignment property be respected or should the alignment be based on the value type
        RotationAngle,              ///< Rotation angle of the cell content, in degrees
        Direction,                  ///< The direction of the text in the cell. This is a CellTextDirection.
        RotationAlign,              ///< How the edge of the text is aligned after rotation. This is a RotationAlignment
        TextWritingMode,            ///< KoText::Direction, the direction for writing text in the cell
        VerticalGlyphOrientation,   ///< bool, specify whether this feature is enabled or not
        CellBackgroundBrush,        ///< the cell background brush, as QTextFormat::BackgroundBrush is used by paragraphs
        VerticalAlignment,          ///< the vertical alignment oinside the cell
        MasterPageName,             ///< Optional name of the master-page
        InlineRdf,                  ///< Optional KoTextInlineRdf object
        Borders,                    ///< KoBorder, the borders of this cell
        Shadow,                     ///< KoShadowStyle, the shadow of this cell
        CellIsProtected             ///< boolean, if true, the cell is protected against edits
                                        /// It's not really a property of KoTableCellStyle but defined here for convenience
        ,LastCellStyleProperty
    };

    /// Constructor
    explicit KoTableCellStyle(QObject *parent = 0);
    /// Creates a KoTableCellStyle with the given table cell format, and \a parent
    explicit KoTableCellStyle(const QTextTableCellFormat &tableCellFormat, QObject *parent = 0);
    KoTableCellStyle(const KoTableCellStyle &other);
    KoTableCellStyle& operator=(const KoTableCellStyle &other);

    /// Destructor
    ~KoTableCellStyle();

    /// Creates a KoTableCellStyle that represents the formatting of \a block.
    static KoTableCellStyle *fromTableCell(const QTextTableCell &table, QObject *parent = 0);

    /// Creates a clean QTextCharFormat, but keeps all the table cell properties.
    /// This is needed since block.charformat doubles as the QTextTableCellFormat
    /// This method works even if \a charFormat is not a QTextTableCellFormat
    static QTextCharFormat cleanCharFormat(const QTextCharFormat &charFormat);

    /// creates a clone of this style with the specified parent
    KoTableCellStyle *clone(QObject *parent = 0);

    /**
     * Adjust the bounding rectange \boundingRect according to the paddings and margins
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

    void setBackground(const QBrush &brush);
    /// See similar named method on QTextBlockFormat
    QBrush background() const;
    /// See similar named method on QTextBlockFormat
    void clearBackground();

    /**
     * Get the paragraph style for this cell style
     *
     * @return the paragraph style
     */
    KoParagraphStyle *paragraphStyle() const;

    bool shrinkToFit() const;
    void setShrinkToFit(bool state);

    bool repeatContent() const;
    void setRepeatContent(bool state);

    void setLeftPadding(qreal padding);
    void setTopPadding(qreal padding);
    void setRightPadding(qreal padding);
    void setBottomPadding(qreal padding);
    void setPadding(qreal padding);

    qreal leftPadding() const;
    qreal rightPadding() const;
    qreal topPadding() const;
    qreal bottomPadding() const;

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;

    KoText::Direction textDirection() const;
    void setTextDirection (KoText::Direction value);

    void setWrap(bool state);
    bool wrap() const;

    CellProtectionFlag cellProtection() const;
    void setCellProtection (CellProtectionFlag protection);

    void setPrintContent(bool state);
    bool printContent() const;

    void setDecimalPlaces(int places);
    int decimalPlaces() const;

    void setAlignFromType(bool state);
    bool alignFromType() const;

    void setRotationAngle(qreal value);
    qreal rotationAngle() const;

    void setDirection(CellTextDirection direction);
    CellTextDirection direction() const;

    void setRotationAlignment(RotationAlignment align);
    RotationAlignment rotationAlignment () const;

    void setVerticalGlyphOrientation(bool state);
    bool verticalGlyphOrientation() const;

    void setBorders(const KoBorder &borders);
    KoBorder borders() const;

    void setShadow (const KoShadowStyle &shadow);
    KoShadowStyle shadow() const;

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
     * Apply this style to a textTableCellFormat by copying all properties from this, and parent
     * styles to the target textTableCellFormat.  Note that the paragraph format will not be applied
     * using this method, use the other method for that.
     * No default values are applied.
     */
    void applyStyle(QTextTableCellFormat &format) const;

    void applyStyle(QTextTableCell &cell) const;

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
    void loadOdf(const KoXmlElement *element, KoShapeLoadingContext &context);

    void saveOdf(KoGenStyle &style, KoShapeSavingContext &context);

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


    /**
     * Set the properties of an edge.
     *
     * @param side defines which edge this is for.
     * @param style the border style for this side.
     * @param totalWidth the thickness of the border. Sum of outerwidth, spacing and innerwidth for double borders
     * @param color the color of the border line(s).
     */
    void setEdge(KoBorder::BorderSide side, KoBorder::BorderStyle style,
                 qreal totalWidth, const QColor &color);

    /**
     * Set the properties of a double border.
     * Note: you need to set the edge first or that would overwrite these values.
     *
     * The values will not be set if the border doesn't have a double style
     *
     * @param side defines which edge this is for.
     * @param space the amount of spacing between the outer border and the inner border in case of style being double
     * @param innerWidth the thickness of the inner border line in case of style being double
     */
    void setEdgeDoubleBorderValues(KoBorder::BorderSide side, qreal innerWidth, qreal space);

    /**
     * Check if the border data has any borders.
     *
     * @return true if there has been at least one border set.
     */
    bool hasBorders() const;

    qreal leftBorderWidth() const;
    qreal rightBorderWidth() const;
    qreal topBorderWidth() const;
    qreal bottomBorderWidth() const;

    qreal leftInnerBorderWidth() const;
    qreal rightInnerBorderWidth() const;
    qreal topInnerBorderWidth() const;
    qreal bottomInnerBorderWidth() const;

    qreal leftOuterBorderWidth() const;
    qreal rightOuterBorderWidth() const;
    qreal topOuterBorderWidth() const;
    qreal bottomOuterBorderWidth() const;

    KoBorder::BorderData getEdge(KoBorder::BorderSide side) const;
    KoBorder::BorderStyle getBorderStyle(KoBorder::BorderSide side) const;
Q_SIGNALS:
    void nameChanged(const QString &newName);

protected:
    KoTableCellStylePrivate * const d_ptr;

private:
    /**
     * Load the style from the \a KoStyleStack style stack using the
     * OpenDocument format.
     */
    void loadOdfProperties(KoShapeLoadingContext &context, KoStyleStack &styleStack);
    qreal propertyDouble(int key) const;
    QPen propertyPen(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QColor propertyColor(int key) const;


    /**
     * Set the format properties from an Edge structure
     *
     * @param side defines which edge this is for.
     * @param style the border style for this side.
     * @param edge the Edge that hold the properties values
     */
    void setEdge(KoBorder::BorderSide side, const KoBorder::BorderData &edge, KoBorder::BorderStyle style);

    Q_DECLARE_PRIVATE(KoTableCellStyle)

};

Q_DECLARE_METATYPE(KoTableCellStyle *)

#endif
