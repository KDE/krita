/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef KOLISTLEVELPROPERTIES_H
#define KOLISTLEVELPROPERTIES_H

#include "KoListStyle.h"

#include <QString>
#include <QChar>
#include <QSharedData>

#include <KoXmlReader.h>

class KoListStyle;
class KoTextLoadingContext;

class KOTEXT_EXPORT KoListLevelProperties {
public:
    /// Constructor
    explicit KoListLevelProperties();
    /// Copy constructor
    KoListLevelProperties(const KoListLevelProperties &other);
    /// Destructor
    ~KoListLevelProperties();

    /// set the style to be used for this list-level.
    void setStyle(KoListStyle::Style style);
    /// return the used style
    KoListStyle::Style style() const;
    /// set the string that will be shown before the counter in the list label
    void setListItemPrefix(const QString &prefix);
    /// return the string that will be shown before the counter in the list label
    QString listItemPrefix() const;
    /// set the string that will be shown after the counter in the list label
    void setListItemSuffix(const QString &suffix);
    /// return the string that will be shown after the counter in the list label
    QString listItemSuffix() const;
    /// set the index of the first value of this whole list.
    void setStartValue(int value);
    /// return the index of the first value of this whole list.
    int startValue() const;
    /// set the list level which is how deep the counter is nested below other lists (should be >=1)
    void setLevel(int level);
    /// return the list level which is how deep the counter is nested below other lists
    int level() const;
    /// set the amount of levels that will be shown in list items of this list.
    void setDisplayLevel(int level);
    /// return the amount of levels that will be shown in list items of this list.
    int displayLevel() const;
    /// set the styleId of the KoCharacterStyle to be used to layout the listitem
    void setCharacterStyleId(int id);
    /// return the styleId of the KoCharacterStyle to be used to layout the listitem
    int characterStyleId() const;
    /// set the character to be used as the counter of the listitem
    void setBulletCharacter(QChar character);
    /// return the character to be used as the counter of the listitem
    QChar bulletCharacter() const;
    /// set the size, in percent, of the bullet counter relative to the fontsize of the counter
    void setRelativeBulletSize(int percent);
    /// return the size, in percent, of the bullet counter relative to the fontsize of the counter
    int relativeBulletSize() const;
    /// set how the list label should be aligned in the width this list reserves for the listitems
    void setAlignment(Qt::Alignment align);
    /// return how the list label should be aligned in the width this list reserves for the listitems
    Qt::Alignment alignment() const;
    /// set the minimum width (in pt) of the list label for all items in this list
    void setMinimumWidth(double width);
    /// return the minimum width (in pt) of the list label for all items in this list
    double minimumWidth();
    /// set the listId used by all list-styles that together make 1 user defined list in an ODF file.
    void setListId(const QString &listId);
    /// return the listId used by all list-styles that together make 1 user defined list in an ODF file.
    QString listId() const;
    bool LetterSynchronization() const;
    void setLetterSynchronization(bool on);

    bool operator==(const KoListLevelProperties &other) const;
    KoListLevelProperties & operator=(const KoListLevelProperties &other);

    /**
     * Create a KoListLevelProperties object from a QTextList instance.
     */
    static KoListLevelProperties fromTextList(QTextList *list);

    /**
     * Apply this style to a QTextListFormat by copying all properties from this style
     * to the target list format.
     */
    void applyStyle(QTextListFormat &format) const;

    /**
     * Load the properties from the \p style using the OpenDocument format.
     */
    void loadOasis(KoTextLoadingContext& context, const KoXmlElement& style);

private:
    void setProperty(int key, const QVariant &value);
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    double propertyDouble(int key) const;
    QString propertyString(int key) const;

    class Private;
    Private * const d;
};

#endif
