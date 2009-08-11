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
#ifndef KOTABLESTYLE_H
#define KOTABLESTYLE_H

#include "KoText.h"
#include "kotext_export.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>

struct Property;
class QTextTable;
class KoStyleStack;
class KoGenStyle;
class KoGenStyles;
#include "KoXmlReaderForward.h"
class KoOdfLoadingContext;

/**
 * A container for all properties for the table wide style.
 * Each table in the main text either is based on a table style, or its not. Where
 * it is based on a table style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoTableStyle.
 * @see KoStyleManager
 */
class KOTEXT_EXPORT KoTableStyle : public QObject
{
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextTableFormat::UserProperty + 1,
        // Linespacing properties
        KeepWithNext,    ///< If true, keep table with next paragraph
        BreakBefore,    ///< If true, insert a frame break before this table
        BreakAfter,     ///< If true, insert a frame break after this table
        MayBreakBetweenRows,     ///< If true, then the table is allowed to break between rows
        ColumnAndRowStyleManager,     ///< QMetaType::VoidStar pointer to a KoColumnAndRowStyleManager
                                                             /// It's not really a property of KoTableStyle but defined here for convinience
        CollapsingBorders,     ///< If true, then the table has collapsing border model
        MasterPageName         ///< Optional name of the master-page
    };

    /// Constructor
    KoTableStyle(QObject *parent = 0);
    /// Creates a KoTableStyle with the given table format, and \a parent
    KoTableStyle(const QTextTableFormat &blockFormat, QObject *parent = 0);
    /// Destructor
    ~KoTableStyle();

    /// Creates a KoTableStyle that represents the formatting of \a block.
    static KoTableStyle *fromTable(const QTextTable &table, QObject *parent = 0);

    /// creates a clone of this style with the specified parent
    KoTableStyle *clone(QObject *parent = 0);

    /// See similar named method on QTextFrameFormat
    void setWidth(const QTextLength &width);

    /// The property specifies if the table should be kept together with the next paragraph
    void setKeepWithNext(bool keep);

    /// The property specifies if the table should allow it to be break. Break within a row is specified per row
    void setMayBreakBetweenRows(bool allow);

    /// See similar named method on QTextBlockFormat
    void setBackground(const QBrush &brush);
    /// See similar named method on QTextBlockFormat
    QBrush background() const;
    /// See similar named method on QTextBlockFormat
    void clearBackground();

    void setBreakBefore(bool on);
    bool breakBefore();
    void setBreakAfter(bool on);
    bool breakAfter();
    
    void setCollapsingBorderModel(bool on);
    bool collapsingBorderModel();

    // ************ properties from QTextTableFormat
    /// duplicated property from QTextBlockFormat
    void setTopMargin(qreal topMargin);
    /// duplicated property from QTextBlockFormat
    qreal topMargin() const;
    /// duplicated property from QTextBlockFormat
    void setBottomMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal bottomMargin() const;
    /// duplicated property from QTextBlockFormat
    void setLeftMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal leftMargin() const;
    /// duplicated property from QTextBlockFormat
    void setRightMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal rightMargin() const;
    /// set the margin around the table, making the margin on all sides equal.
    void setMargin(qreal margin);

    /// duplicated property from QTextBlockFormat
    void setAlignment(Qt::Alignment alignment);
    /// duplicated property from QTextBlockFormat
    Qt::Alignment alignment() const;

    /// set the parent style this one inherits its unset properties from.
    void setParentStyle(KoTableStyle *parent);

    /// return the parent style
    KoTableStyle *parentStyle() const;

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
    void copyProperties(const KoTableStyle *style);

    /**
     * Apply this style to a tableFormat by copying all properties from this, and parent
     * styles to the target table format.
     */
    void applyStyle(QTextTableFormat &format) const;

    void remove(int key);

    /// Compare the properties of this style with the other
    bool operator==(const KoTableStyle &other) const;

    void removeDuplicates(const KoTableStyle &other);

    /**
     * Load the style form the element
     *
     * @param context the odf loading context
     * @param element the element containing the
     */
    void loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context);

    void saveOdf(KoGenStyle &style);

    /**
     * Returns true if this table style has the property set.
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
    Qt::Alignment alignmentFromString(const QString &align);
    QString alignmentToString(Qt::Alignment alignment);
    qreal propertyDouble(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QColor propertyColor(int key) const;

    class Private;
    Private * const d;
};

#endif
