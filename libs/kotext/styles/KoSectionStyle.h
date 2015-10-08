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
#ifndef KOSECTIONSTYLE_H
#define KOSECTIONSTYLE_H

#include "KoColumns.h"
#include "KoText.h"
#include "kritatext_export.h"

#include <QObject>
#include <QTextFormat>

class QTextFrame;
class QTextFrameFormat;
class KoGenStyle;
class KoOdfLoadingContext;

class QString;
class QVariant;

/**
 * A container for all properties for the section wide style.
 * Each section in the main text either is based on a section style, or its not. Where
 * it is based on a section style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoSectionStyle.
 * @see KoStyleManager
 */
class KRITATEXT_EXPORT KoSectionStyle : public QObject
{
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextFormat::UserProperty + 1,
        TextProgressionDirection,
        ColumnCount,
        ColumnData,
        ColumnGapWidth,
        SeparatorStyle,
        SeparatorColor,
        SeparatorVerticalAlignment,
        SeparatorWidth,
        SeparatorHeight
    };

    /// Constructor
    explicit KoSectionStyle(QObject *parent = 0);
    /// Creates a KoSectionStyle with the given frame format and \a parent
    explicit KoSectionStyle(const QTextFrameFormat &frameFormat, QObject *parent = 0);
    /// Destructor
    ~KoSectionStyle();

    /// creates a clone of this style with the specified parent
    KoSectionStyle *clone(QObject *parent = 0) const;


    /// duplicated property from QTextBlockFormat
    void setLeftMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal leftMargin() const;
    /// duplicated property from QTextBlockFormat
    void setRightMargin(qreal margin);
    /// duplicated property from QTextBlockFormat
    qreal rightMargin() const;

    KoText::Direction textProgressionDirection() const;
    
    void setTextProgressionDirection(KoText::Direction dir);
    
    /// See similar named method on QTextBlockFormat
    void setBackground(const QBrush &brush);
    /// See similar named method on QTextBlockFormat
    QBrush background() const;
    /// See similar named method on QTextBlockFormat
    void clearBackground();

#if 0
    as this is a duplicate of leftMargin, lets make it very clear we are using that one.
    /// duplicated property from QTextBlockFormat
    void setIndent(int indent);
    /// duplicated property from QTextBlockFormat
    int indent() const;
#endif

    void setColumnCount(int columnCount);
    int columnCount() const;

    void setColumnGapWidth(qreal columnGapWidth);
    qreal columnGapWidth() const;

    void setColumnData(const QList<KoColumns::ColumnDatum> &columnData);
    QList<KoColumns::ColumnDatum> columnData() const;

    void setSeparatorStyle(KoColumns::SeparatorStyle separatorStyle);
    KoColumns::SeparatorStyle separatorStyle() const;

    void setSeparatorColor(const QColor &separatorColor);
    QColor separatorColor() const;

    void setSeparatorVerticalAlignment(KoColumns::SeparatorVerticalAlignment separatorVerticalAlignment);
    KoColumns::SeparatorVerticalAlignment separatorVerticalAlignment() const;

    void setSeparatorWidth(qreal separatorWidth);
    qreal separatorWidth() const;

    void setSeparatorHeight(int separatorHeight);
    int separatorHeight() const;

    /// set the parent style this one inherits its unset properties from.
    void setParentStyle(KoSectionStyle *parent);

    /// return the parent style
    KoSectionStyle *parentStyle() const;

    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /// copy all the properties from the other style to this style, effectively duplicating it.
    void copyProperties(const KoSectionStyle *style);

    void unapplyStyle(QTextFrame &section) const;

    /**
     * Apply this style to a frameFormat by copying all properties from this, and parent
     * styles to the target frame format.  Note that the character format will not be applied
     * using this method, use the other applyStyle() method for that.
     */
    void applyStyle(QTextFrameFormat &format) const;

    /**
     * Apply this style to the section (QTextFrame) by copying all properties from this and parent
     * to the target frame formats.
     */
    void applyStyle(QTextFrame &section) const;

    void remove(int key);

    /// Compare the section of this style with the other
    bool operator==(const KoSectionStyle &other) const;
    /// Compare the section properties of this style with other
    bool compareSectionProperties(const KoSectionStyle &other) const;

    void removeDuplicates(const KoSectionStyle &other);

    /**
     * Load the style form the element
     *
     * @param context the odf loading context
     * @param element the element containing the
     */
    void loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context);

    void saveOdf(KoGenStyle &style);

    /**
     * Returns true if this section style has the property set.
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
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoSectionStyle *)

#endif
