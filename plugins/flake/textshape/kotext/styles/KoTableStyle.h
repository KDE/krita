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
#ifndef KOTABLESTYLE_H
#define KOTABLESTYLE_H

#include "KoText.h"
#include "kritatext_export.h"
#include <KoXmlReaderForward.h>

#include <QObject>

class KoStyleStack;
class KoGenStyle;
class KoShadowStyle;

class KoOdfLoadingContext;

class QTextTable;
class QVariant;

/**
 * A container for all properties for the table wide style.
 * Each table in the main text either is based on a table style, or its not. Where
 * it is based on a table style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoTableStyle.
 * @see KoStyleManager
 */
class KRITATEXT_EXPORT KoTableStyle : public QObject
{
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextTableFormat::UserProperty + 100,
        // Linespacing properties
        KeepWithNext,               ///< If true, keep table with next paragraph
        BreakBefore,                ///< If true, insert a frame break before this table
        BreakAfter,                 ///< If true, insert a frame break after this table
        MayBreakBetweenRows,        ///< If true, then the table is allowed to break between rows
        ColumnAndRowStyleManager,   ///< QVariant of a KoColumnAndRowStyleManager
                                        /// It's not really a property of KoTableStyle but defined here for convenience
        CollapsingBorders,          ///< If true, then the table has collapsing border model
        MasterPageName,             ///< Optional name of the master-page
        NumberHeadingRows,          ///< Count the number of heading rows
        Visible,                    ///< If true, the table is visible
        PageNumber,                 ///< The page number that is applied after the page break
        TextProgressionDirection,   ///< The direction of the text in the table
        TableIsProtected,           ///< boolean, if true, the table is protected against edits
                                        /// It's not really a property of KoTableStyle but defined here for convenience
        Shadow,                      ///< KoShadowStyle, the table shadow
        TableTemplate,               ///< KoTextTableTemplate, template for the table
        UseBandingColumnStyles,      ///< table:use-banding-column-styles ODF 1.2 19.736
        UseBandingRowStyles,         ///< table:use-banding-row-styles ODF 1.2 19.737
        UseFirstColumnStyles,        ///< table:use-first-column-styles ODF 1.2 19.738
        UseFirstRowStyles,           ///< table:use-first-row-styles ODF 1.2 19.739
        UseLastColumnStyles,         ///< table:use-last-column-styles ODF 1.2 19.740
        UseLastRowStyles             ///< table:use-last-row-styles ODF 1.2 19.741
    };

    /// Constructor
    explicit KoTableStyle(QObject *parent = 0);
    /// Creates a KoTableStyle with the given table format, and \a parent
    explicit KoTableStyle(const QTextTableFormat &blockFormat, QObject *parent = 0);
    /// Destructor
    ~KoTableStyle();

    /// Creates a KoTableStyle that represents the formatting of \a table.
    static KoTableStyle *fromTable(const QTextTable &table, QObject *parent = 0);

    /// creates a clone of this style with the specified parent
    KoTableStyle *clone(QObject *parent = 0);

    /// See similar named method on QTextFrameFormat
    void setWidth(const QTextLength &width);

    /// The property specifies if the table should be kept together with the next paragraph
    void setKeepWithNext(bool keep);
    
    bool keepWithNext() const;

    /// This property describe the shadow of the table, if any
    void setShadow (const KoShadowStyle &shadow);

    KoShadowStyle shadow() const;

    /// The property specifies if the table should allow it to be break. Break within a row is specified per row
    void setMayBreakBetweenRows(bool allow);
    bool mayBreakBetweenRows() const;

    /// See similar named method on QTextBlockFormat
    void setBackground(const QBrush &brush);
    /// See similar named method on QTextBlockFormat
    QBrush background() const;
    /// See similar named method on QTextBlockFormat
    void clearBackground();
    
    int pageNumber() const;
    void setPageNumber (int page);

    void setBreakBefore(KoText::KoTextBreakProperty state);
    KoText::KoTextBreakProperty breakBefore() const;
    void setBreakAfter(KoText::KoTextBreakProperty state);
    KoText::KoTextBreakProperty breakAfter() const;

    void setVisible(bool on);
    bool visible() const;

    void setCollapsingBorderModel(bool on);
    bool collapsingBorderModel() const;

    KoText::Direction textDirection() const;
    void setTextDirection(KoText::Direction direction);

    // ************ properties from QTextTableFormat
    /// duplicated property from QTextBlockFormat
    void setTopMargin(QTextLength topMargin);
    /// duplicated property from QTextBlockFormat
    qreal topMargin() const;
    /// duplicated property from QTextBlockFormat
    void setBottomMargin(QTextLength margin);
    /// duplicated property from QTextBlockFormat
    qreal bottomMargin() const;
    /// duplicated property from QTextBlockFormat
    void setLeftMargin(QTextLength margin);
    /// duplicated property from QTextBlockFormat
    qreal leftMargin() const;
    /// duplicated property from QTextBlockFormat
    void setRightMargin(QTextLength margin);
    /// duplicated property from QTextBlockFormat
    qreal rightMargin() const;
    /// set the margin around the table, making the margin on all sides equal.
    void setMargin(QTextLength margin);

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
    
    /// return true when there are keys defined for this style
    bool isEmpty() const;

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

Q_SIGNALS:
    void nameChanged(const QString &newName);

private:
    /**
     * Load the style from the \a KoStyleStack style stack using the
     * OpenDocument format.
     */
    void loadOdfProperties(KoStyleStack &styleStack);
    static Qt::Alignment alignmentFromString(const QString &align);
    static QString alignmentToString(Qt::Alignment alignment);
    qreal propertyDouble(int key) const;
    QTextLength propertyLength(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QColor propertyColor(int key) const;

    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoTableStyle *)

#endif
