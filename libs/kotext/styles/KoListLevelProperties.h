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

class KoListStyle;

class KOTEXT_EXPORT KoListLevelProperties {
public:
    KoListLevelProperties();
    KoListLevelProperties(const KoListLevelProperties &other);
    ~KoListLevelProperties();

    /// set the style to be used for this list-level.
    void setStyle(KoListStyle::Style style);
    /// return the used style
    KoListStyle::Style style() const;
    void setListItemPrefix(const QString &prefix);
    QString listItemPrefix() const;
    void setListItemSuffix(const QString &suffix);
    QString listItemSuffix() const;
    void setStartValue(int value);
    int startValue() const;
    void setLevel(int level);
    int level() const;
    void setDisplayLevel(int level);
    int displayLevel() const;
    void setCharacterStyleId(int id);
    int characterStyleId() const;
    void setBulletCharacter(QChar character);
    QChar bulletCharacter() const;
    void setRelativeBulletSize(int percent);
    int relativeBulletSize() const;
    void setAlignment(Qt::Alignment align);
    Qt::Alignment alignment() const;
    void setMinimumWidth(double width);
    double minimumWidth();

    bool operator==(const KoListLevelProperties &other) const;
    KoListLevelProperties & operator=(const KoListLevelProperties &other);


    static KoListLevelProperties fromTextList(QTextList *list);

    /**
     * Apply this style to a QTextListFormat by copying all properties from this style
     * to the target list format.
     */
    void applyStyle(QTextListFormat &format) const;

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
