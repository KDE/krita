/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#ifndef KOLISTSTYLE_H
#define KOLISTSTYLE_H

#include <QTextFormat>
#include <QMap>
#include <QPointer>
#include <QTextList>

class StylePrivate;


/**
 * This class groups all styling-options for lists.
 */
class KoListStyle {
public:
    enum Style {
        SquareItem = QTextListFormat::ListSquare,
        DiscItem = QTextListFormat::ListDisc,
        CircleItem = QTextListFormat::ListCircle,
        DecimalItem = QTextListFormat::ListDecimal,
        AlphaLowerItem = QTextListFormat::ListLowerAlpha,
        UpperAlphaItem = QTextListFormat::ListUpperAlpha,
        NoItem = 1,
        RomanLowerItem,
        UpperRomanItem,
        BoxItem,
        CustomCharItem
         // TODO look at css 3 for things like hebrew counters
    };

    enum Property {
        ListItemPrefix = QTextFormat::UserProperty+1000, ///< The text to be printed before the listItem
        ListItemSuffix, ///< The text to be printed after the listItem
        ConsecutiveNumbering,   ///< when true don't let non-numbering parags restart numbering
        StartValue,     ///< First value to use
        Level,          ///< list nesting level, is 1 or higher, or zero when implied
        DisplayLevel,   ///< show this many levels. Is always lower than the (implied) level.
        CharacterStyleId,///< CharacterStyle used for markup of the counter
        BulletCharacter,///< an int with the unicode value of the character
        BulletSize,     ///< size in percent relative to the height of the text
        ListHeader,     ///< TextBlock level flag to mark a block to be a listHeader
        NoListItem,     ///< TextBlock level flag to mark a block to not have a listItem
        ExplicitListValue ///< TextBlock level int with the value that that parag will have
    };

    KoListStyle();
    KoListStyle(const KoListStyle &orig);

    void setStyle(Style style) { setProperty(QTextListFormat::ListStyle, (int) style); }
    Style style() const { return static_cast<Style> (propertyInt(QTextListFormat::ListStyle)); }
    void setListItemPrefix(const QString &prefix) { setProperty(ListItemPrefix, prefix ); }
    QString listItemPrefix() const { return propertyString(ListItemPrefix); }
    void setListItemSuffix(const QString &suffix) { setProperty(ListItemSuffix, suffix  ); }
    QString listItemSuffix() const { return propertyString(ListItemSuffix); }
    /**
     * If true keep numbering even if there was a lower list in between listitems.
     * This attribute specifies whether or not the list style uses consecutive numbering for
     * all list levels or whether each list level restarts the numbering.
     */
    void setConsecutiveNumbering(bool on) { setProperty(ConsecutiveNumbering, on  ); }
    /**
     * If true keep numbering even if there was a lower list in between listitems.
     * This attribute specifies whether or not the list style uses consecutive numbering for
     * all list levels or whether each list level restarts the numbering.
     */
    bool consecutiveNumbering() const { return propertyBoolean (ConsecutiveNumbering); }
    void setStartValue(int value) { setProperty(StartValue, value  ); }
    int startValue() const { return propertyInt (StartValue); }
    void setLevel(int level) { setProperty(Level, level  ); }
    int level() const { return propertyInt (Level); }
    void setDisplayLevel(int level) { setProperty(DisplayLevel, level  ); }
    int displayLevel() const { return propertyInt (DisplayLevel); }
    void setCharacterStyleId(int id) { setProperty(CharacterStyleId, id  ); }
    int characterStyleId() const { return propertyInt (CharacterStyleId); }
    void setBulletCharacter(QChar character) { setProperty(BulletCharacter, (int) character.unicode() ); }
    QChar bulletCharacter() const { return propertyInt (BulletCharacter); }
    void setRelativeBulletSize(int percent) { setProperty(BulletSize, percent  ); }
    int relativeBulletSize() const { return propertyInt (BulletSize); }

    /// return the name of the style.
    const QString& name() const { return m_name; }

    /// set a user-visible name on the style.
    void setName(const QString &name) { m_name = name; }

    /**
     * Apply this style to a blockFormat by copying all properties from this style
     * to the target block format.
     */
    void applyStyle(const QTextBlock &block);

protected:
    friend class KoParagraphStyle;
    void addUser() { m_refCount++; }
    void removeUser() { m_refCount--; }
    int userCount() const { return m_refCount; }

    void apply(const KoListStyle &other);

private:
    void setProperty(int key, const QVariant &value);
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QString propertyString(int key) const;

private:
    QString m_name;
    StylePrivate *m_stylesPrivate;
    QMap<const QTextDocument*, QPointer<QTextList> > m_textLists;
    int m_refCount;
};

#endif
