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

#include "KoListLevelProperties.h"
#include "Styles_p.h"
//#include "KoTextBlockData.h"

//#include <QTextCursor>
//#include <QSharedData>

class KoListLevelProperties::Private {
public:
    StylePrivate stylesPrivate;
    //QMap<const QTextDocument*, QPointer<QTextList> > textLists;

    void copy(Private *other) {
        stylesPrivate = other->stylesPrivate;
        //textLists = other->textLists;
    }
};

// all relevant properties.
static const int properties[] = {
    QTextListFormat::ListStyle,
    KoListStyle::ListItemPrefix,
    KoListStyle::ListItemSuffix,
    KoListStyle::StartValue,
    KoListStyle::Level,
    KoListStyle::DisplayLevel,
    KoListStyle::CharacterStyleId,
    KoListStyle::BulletCharacter,
    KoListStyle::BulletSize,
    KoListStyle::Alignment,
    -1
};

KoListLevelProperties::KoListLevelProperties()
    : d( new Private())
{
    setStyle(KoListStyle::DiscItem);
    setStartValue(1);
}

KoListLevelProperties::KoListLevelProperties(const KoListLevelProperties &other)
    : d( new Private())
{
    d->copy(other.d);
}

KoListLevelProperties::~KoListLevelProperties() {
    delete d;
}

void KoListLevelProperties::setProperty(int key, const QVariant &value) {
    d->stylesPrivate.add(key, value);
}

int KoListLevelProperties::propertyInt(int key) const {
    QVariant variant = d->stylesPrivate.value(key);
    if(variant.isNull())
        return 0;
    return variant.toInt();
}

double KoListLevelProperties::propertyDouble(int key) const {
    QVariant variant = d->stylesPrivate.value(key);
    if(variant.isNull())
        return 0.;
    return variant.toDouble();
}

bool KoListLevelProperties::propertyBoolean(int key) const {
    QVariant variant = d->stylesPrivate.value(key);
    if(variant.isNull())
        return false;
    return variant.toBool();
}

QString KoListLevelProperties::propertyString(int key) const {
    QVariant variant = d->stylesPrivate.value(key);
    if(variant.isNull())
        return QString();
    return qvariant_cast<QString>(variant);
}

void KoListLevelProperties::applyStyle(QTextListFormat &format) const {
    int i=0;
    while(properties[i] != -1) {
        QVariant variant = d->stylesPrivate.value(properties[i]);
        if(! variant.isNull())
            format.setProperty(properties[i], variant);
        i++;
    }
}

bool KoListLevelProperties::operator==(const KoListLevelProperties &other) const {
    // TODO move this to the stylesPrivate

    int i=0;
    while(properties[i] != -1) {
        QVariant variant = d->stylesPrivate.value(properties[i]);
        if(! variant.isNull()) {
            if(other.d->stylesPrivate.value(properties[i]) != 0)
                return false;
        }
        else {
            QVariant otherVariant = d->stylesPrivate.value(properties[i]);
            if(otherVariant == 0 || otherVariant != variant)
                return false;
        }
        i++;
    }
    return true;
}

void KoListLevelProperties::setListItemPrefix(const QString &prefix) {
    setProperty(KoListStyle::ListItemPrefix, prefix );
}

QString KoListLevelProperties::listItemPrefix() const {
    return propertyString(KoListStyle::ListItemPrefix);
}

void KoListLevelProperties::setStyle(KoListStyle::Style style) {
    setProperty(QTextListFormat::ListStyle, (int) style);
}

KoListStyle::Style KoListLevelProperties::style() const {
    return static_cast<KoListStyle::Style> (propertyInt(QTextListFormat::ListStyle));
}

void KoListLevelProperties::setListItemSuffix(const QString &suffix) {
    setProperty(KoListStyle::ListItemSuffix, suffix  );
}

QString KoListLevelProperties::listItemSuffix() const {
    return propertyString(KoListStyle::ListItemSuffix);
}

void KoListLevelProperties::setStartValue(int value) {
    setProperty(KoListStyle::StartValue, value  );
}

int KoListLevelProperties::startValue() const {
    return propertyInt (KoListStyle::StartValue);
}

void KoListLevelProperties::setLevel(int value) {
    setProperty(KoListStyle::Level, value  );
}

int KoListLevelProperties::level() const {
    return propertyInt (KoListStyle::Level);
}

void KoListLevelProperties::setDisplayLevel(int level) {
    setProperty(KoListStyle::DisplayLevel, level  );
}

int KoListLevelProperties::displayLevel() const {
    return propertyInt (KoListStyle::DisplayLevel);
}

void KoListLevelProperties::setCharacterStyleId(int id) {
    setProperty(KoListStyle::CharacterStyleId, id  );
}

int KoListLevelProperties::characterStyleId() const {
    return propertyInt (KoListStyle::CharacterStyleId);
}

void KoListLevelProperties::setBulletCharacter(QChar character) {
    setProperty(KoListStyle::BulletCharacter, (int) character.unicode() );
}

QChar KoListLevelProperties::bulletCharacter() const {
    return propertyInt (KoListStyle::BulletCharacter);
}

void KoListLevelProperties::setRelativeBulletSize(int percent) {
    setProperty(KoListStyle::BulletSize, percent  );
}

int KoListLevelProperties::relativeBulletSize() const {
    return propertyInt (KoListStyle::BulletSize);
}

void KoListLevelProperties::setAlignment(Qt::Alignment align) {
    setProperty(KoListStyle::Alignment, static_cast<int> (align) );
}

Qt::Alignment KoListLevelProperties::alignment() const {
    return static_cast<Qt::Alignment>(propertyInt(KoListStyle::Alignment));
}

void KoListLevelProperties::setMinimumWidth(double width) {
    setProperty(KoListStyle::MinimumWidth, width);
}

double KoListLevelProperties::minimumWidth() {
    return propertyDouble(KoListStyle::MinimumWidth);
}

KoListLevelProperties & KoListLevelProperties::operator=(const KoListLevelProperties &other) {
    d->copy(other.d);
    return *this;
}

// static
KoListLevelProperties KoListLevelProperties::fromTextList(QTextList *list) {
    KoListLevelProperties llp;
    QTextListFormat format = list->format();
    int i=0;
    while(properties[i] != -1) {
        int key = properties[i];
        if(format.hasProperty(key))
            llp.setProperty(key, format.property(key));
        i++;
    }
    return llp;
}
