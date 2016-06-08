/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>
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
#ifndef KOTABLECOLUMNSTYLE_H
#define KOTABLECOLUMNSTYLE_H

#include "KoText.h"
#include "kritatext_export.h"
#include <KoXmlReaderForward.h>

#include <QTextTableFormat>

class KoStyleStack;
class KoGenStyle;
class KoOdfLoadingContext;

class QString;
class QVariant;

/**
 * A container for all properties for the table column style.
 *
 * Named column styles are stored in the KoStyleManager and automatic ones in the
 * KoTableColumnAndRowStyleManager.
 *
 * The style has a property 'StyleId' with an integer as value. The integer value
 * corresponds to the styleId() output of a specific KoTableColumnStyle.
 * *
 * @see KoStyleManager, KoTableRowAndColumnStyleManager
 */
class KRITATEXT_EXPORT KoTableColumnStyle
{
public:
    enum Property {
        StyleId = QTextTableFormat::UserProperty + 1,
        ColumnWidth,         ///< Column width.
        RelativeColumnWidth, ///< Relative column width.
        OptimalColumnWidth,  ///< Use optimal column width
        BreakBefore,         ///< If true, insert a frame break before this table
        BreakAfter,          ///< If true, insert a frame break after this table
        MasterPageName       ///< Optional name of the master-page
    };

    /// Constructor
    KoTableColumnStyle();
    /// Constructor
    KoTableColumnStyle(const KoTableColumnStyle &rhs);
    /// assign operator
    KoTableColumnStyle &operator=(const KoTableColumnStyle &rhs);

    /// Destructor
    ~KoTableColumnStyle();

    void copyProperties(const KoTableColumnStyle *style);

    KoTableColumnStyle *clone() const;

    /// Set the column width.
    void setColumnWidth(qreal width);

    /// Get the column width.
    qreal columnWidth() const;

    /// Set the column width.
    void setRelativeColumnWidth(qreal width);

    /// Get the column width.
    qreal relativeColumnWidth() const;

    /// Get the optimalColumnWidth state
    bool optimalColumnWidth() const;

    /// Set the optimalColumnWidth state
    void setOptimalColumnWidth(bool state);

    /// Set break before. See §7.19.2 of [XSL].
    void setBreakBefore(KoText::KoTextBreakProperty state);

    /// Get break before. See §7.19.2 of [XSL].
    KoText::KoTextBreakProperty breakBefore() const;

    /// Set break after. See §7.19.1 of [XSL].
    void setBreakAfter(KoText::KoTextBreakProperty state);

    /// Get break after. See §7.19.1 of [XSL].
    KoText::KoTextBreakProperty breakAfter() const;

    /// Set the parent style this one inherits its unset properties from.
    void setParentStyle(KoTableColumnStyle *parent);

    /// Return the parent style.
    KoTableColumnStyle *parentStyle() const;

    /// Return the name of the style.
    QString name() const;

    /// Set a user-visible name on the style.
    void setName(const QString &name);

    /// Each style has a unique ID (non persistent) given out by the style manager.
    int styleId() const;

    /// Each style has a unique ID (non persistent) given out by the styleManager.
    void setStyleId(int id);

    /**
     * Return the optional name of the master-page or a QString() if this paragraph
     * isn't attached to a master-page.
     */
    QString masterPageName() const;

    /// Set the name of the master-page.
    void setMasterPageName(const QString &name);

    /// Remove the property \key from this style.
    void remove(int key);

    /// Remove properties in this style that are already in other.
    void removeDuplicates(const KoTableColumnStyle &other);

    /// Compare the properties of this style with the other.
    bool operator==(const KoTableColumnStyle &other) const;

    /// Compare the properties of this style with the other.
    bool operator!=(const KoTableColumnStyle &other) const;

    /**
     * Load the style form the element
     *
     * @param context the odf loading context
     * @param element the element containing the
     */
    void loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context);

    void saveOdf(KoGenStyle &style) const;

    /**
     * Returns true if this table column style has the property set.
     * Note that this method does not delegate to the parent style.
     * @param key the key as found in the Property enum
     */
    bool hasProperty(int key) const;

    /**
     * Set a property with key to a certain value, overriding the value from the parent style.
     * If the value set is equal to the value of the parent style, the key will be removed instead.
     *
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

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_METATYPE(KoTableColumnStyle *)

#endif
