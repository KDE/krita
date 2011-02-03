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

#include "KoTableBorderStyle.h"
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
class KoTableCellStylePrivate;

/**
 * A container for all properties for the table cell style.
 * Each tablecell in the main text either is based on a table cell style, or its not. Where
 * it is based on a table cell style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoTableCellStyle.
 * @see KoStyleManager
 */
class KOTEXT_EXPORT KoTableCellStyle : public KoTableBorderStyle
{
    Q_OBJECT
public:
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


    /**
     * Paint the background.
     *
     * @painter the painter to draw with.
     * @bounds the bounding rectangle to draw.
     */
    void paintBackground(QPainter &painter, const QRectF &bounds) const;

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

    qreal leftPadding() const;
    qreal rightPadding() const;
    qreal topPadding() const;
    qreal bottomPadding() const;

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;

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

    Q_DECLARE_PRIVATE(KoTableCellStyle)
};

#endif
