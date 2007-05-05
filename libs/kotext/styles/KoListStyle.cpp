/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoListStyle.h"
#include "Styles_p.h"
#include "KoTextBlockData.h"

#include <QTextBlock>
#include <QTextCursor>
#include <kdebug.h>

class KoListStyle::Private {
public:
    Private() : stylesPrivate( new StylePrivate()), refCount(0) { }
    ~Private() {
        delete stylesPrivate;
    }
    QString name;
    StylePrivate *stylesPrivate;
    QMap<const QTextDocument*, QPointer<QTextList> > textLists;
    int refCount;
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

KoListStyle::KoListStyle()
    : d( new Private())
{
    setStyle(DiscItem);
    setStartValue(1);
    setLevel(1);
}

KoListStyle::KoListStyle(const KoListStyle &orig)
    : d( new Private())
{
    d->stylesPrivate->copyMissing(orig.d->stylesPrivate);
    d->name = orig.name();
}

KoListStyle::~KoListStyle() {
    delete d;
}

void KoListStyle::setProperty(int key, const QVariant &value) {
    d->stylesPrivate->add(key, value);
}

int KoListStyle::propertyInt(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return 0;
    return variant.toInt();
}

double KoListStyle::propertyDouble(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return 0.;
    return variant.toDouble();
}

bool KoListStyle::propertyBoolean(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return false;
    return variant.toBool();
}

QString KoListStyle::propertyString(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return QString();
    return qvariant_cast<QString>(variant);
}

void KoListStyle::applyStyle(const QTextBlock &block) {
    QTextList *textList = d->textLists.value(block.document());
    if(textList && block.textList() && block.textList() != textList) // remove old one
        block.textList()->remove(block);
    if(block.textList() == 0 && textList) // add if new
        textList->add(block);
    if(block.textList() && textList == 0) {
        textList = block.textList(); // missed it ?
        d->textLists.insert(block.document(), QPointer<QTextList>(textList));
    }

    QTextListFormat format;
    if(block.textList())
        format = block.textList()->format();

    int i=0;
    while(properties[i] != -1) {
        QVariant variant = d->stylesPrivate->value(properties[i]);
        if(! variant.isNull())
            format.setProperty(properties[i], variant);
        i++;
    }

    if(textList) {
        textList->setFormat(format);
        QTextBlock tb = textList->item(0);
        if(tb.isValid()) { // invalidate the counter part
            KoTextBlockData *userData = dynamic_cast<KoTextBlockData*> (tb.userData());
            if(userData)
                userData->setCounterWidth(-1.0);
        }
    }
    else { // does not exist yet, this is the first parag that uses it :)
        QTextCursor cursor(block);
        textList = cursor.createList(format);
        d->textLists.insert(block.document(), QPointer<QTextList>(textList));
    }
}

void KoListStyle::apply(const KoListStyle &other) {
    d->name = other.name();
    d->stylesPrivate->clearAll();
    d->stylesPrivate->copyMissing(other.d->stylesPrivate);
}

bool KoListStyle::operator==(const KoListStyle &other) const {
    int i=0;
    while(properties[i] != -1) {
        QVariant variant = d->stylesPrivate->value(properties[i]);
        if(! variant.isNull()) {
            if(other.d->stylesPrivate->value(properties[i]) != 0)
                return false;
        }
        else {
            QVariant otherVariant = d->stylesPrivate->value(properties[i]);
            if(otherVariant == 0 || otherVariant != variant)
                return false;
        }
        i++;
    }
    return true;
}

QTextList *KoListStyle::textList(const QTextDocument *doc) {
    return d->textLists[doc];
}

void KoListStyle::setListItemPrefix(const QString &prefix) {
    setProperty(ListItemPrefix, prefix );
}

QString KoListStyle::listItemPrefix() const {
    return propertyString(ListItemPrefix);
}


void KoListStyle::setStyle(KoListStyle::Style style) {
    setProperty(QTextListFormat::ListStyle, (int) style);
}
KoListStyle::Style KoListStyle::style() const {
    return static_cast<Style> (propertyInt(QTextListFormat::ListStyle));
}
void KoListStyle::setListItemSuffix(const QString &suffix) {
    setProperty(ListItemSuffix, suffix  );
}
QString KoListStyle::listItemSuffix() const {
    return propertyString(ListItemSuffix);
}
void KoListStyle::setStartValue(int value) {
    setProperty(StartValue, value  );
}
int KoListStyle::startValue() const {
    return propertyInt (StartValue);
}
void KoListStyle::setLevel(int level) {
    setProperty(Level, level  );
}
int KoListStyle::level() const {
    return propertyInt (Level);
}
void KoListStyle::setDisplayLevel(int level) {
    setProperty(DisplayLevel, level  );
}
int KoListStyle::displayLevel() const {
    return propertyInt (DisplayLevel);
}
void KoListStyle::setCharacterStyleId(int id) {
    setProperty(CharacterStyleId, id  );
}
int KoListStyle::characterStyleId() const {
    return propertyInt (CharacterStyleId);
}
void KoListStyle::setBulletCharacter(QChar character) {
    setProperty(BulletCharacter, (int) character.unicode() );
}
QChar KoListStyle::bulletCharacter() const {
    return propertyInt (BulletCharacter);
}
void KoListStyle::setRelativeBulletSize(int percent) {
    setProperty(BulletSize, percent  );
}
int KoListStyle::relativeBulletSize() const {
    return propertyInt (BulletSize);
}
void KoListStyle::setAlignment(Qt::Alignment align) {
    setProperty(Alignment, static_cast<int> (align) );
}
Qt::Alignment KoListStyle::alignment() const {
    return static_cast<Qt::Alignment>(propertyInt(Alignment));
}
void KoListStyle::setMinimumWidth(double width) {
    setProperty(MinimumWidth, width);
}
double KoListStyle::minimumWidth() {
    return propertyDouble(MinimumWidth);
}
QString KoListStyle::name() const {
    return d->name;
}
void KoListStyle::setName(const QString &name) {
    d->name = name;
}
void KoListStyle::addUser() {
    d->refCount++;
}
void KoListStyle::removeUser() {
    d->refCount--;
}
int KoListStyle::userCount() const {
    return d->refCount;
}


// static
KoListStyle* KoListStyle::fromTextList(QTextList *list) {
    KoListStyle *answer = new KoListStyle();

    QTextListFormat format = list->format();
    int i=0;
    while(properties[i] != -1) {
        int key = properties[i];
        if(format.hasProperty(key))
            answer->setProperty(key, format.property(key));
        i++;
    }

    return answer;
}
