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

// all relevant properties.
static const int properties[] = {
    QTextListFormat::ListStyle,
    KoListStyle::ListItemPrefix,
    KoListStyle::ListItemSuffix,
    KoListStyle::ConsecutiveNumbering,
    KoListStyle::StartValue,
    KoListStyle::Level,
    KoListStyle::DisplayLevel,
    KoListStyle::CharacterStyleId,
    KoListStyle::BulletCharacter,
    KoListStyle::BulletSize,
    KoListStyle::Alignment,
    -1
};

KoListStyle::KoListStyle() {
    m_refCount = 0;
    m_stylesPrivate = new StylePrivate();
    setStyle(DiscItem);
    setStartValue(1);
    setLevel(1);
}

KoListStyle::KoListStyle(const KoListStyle &orig) {
    m_refCount = 0;
    m_stylesPrivate = new StylePrivate();
    m_stylesPrivate->copyMissing(orig.m_stylesPrivate);
    m_name = orig.name();
}

KoListStyle::~KoListStyle() {
    delete m_stylesPrivate;
    m_stylesPrivate = 0;
}

void KoListStyle::setProperty(int key, const QVariant &value) {
    m_stylesPrivate->add(key, value);
}

int KoListStyle::propertyInt(int key) const {
    const QVariant *variant = m_stylesPrivate->get(key);
    if(variant == 0)
        return 0;
    return variant->toInt();
}

double KoListStyle::propertyDouble(int key) const {
    const QVariant *variant = m_stylesPrivate->get(key);
    if(variant == 0)
        return 0.;
    return variant->toDouble();
}

bool KoListStyle::propertyBoolean(int key) const {
    const QVariant *variant = m_stylesPrivate->get(key);
    if(variant == 0)
        return false;
    return variant->toBool();
}

QString KoListStyle::propertyString(int key) const {
    const QVariant *variant = m_stylesPrivate->get(key);
    if(variant == 0)
        return QString();
    return qvariant_cast<QString>(variant);
}

void KoListStyle::applyStyle(const QTextBlock &block) {
    QTextList *textList = m_textLists.value(block.document());
    if(textList && block.textList() && block.textList() != textList) // remove old one
        block.textList()->remove(block);
    if(block.textList() == 0 && textList) // add if new
        textList->add(block);
    if(block.textList() && textList == 0) {
        textList = block.textList(); // missed it ?
        m_textLists.insert(block.document(), QPointer<QTextList>(textList));
    }

    QTextListFormat format;
    if(block.textList())
        format = block.textList()->format();

    int i=0;
    while(properties[i] != -1) {
        QVariant const *variant = m_stylesPrivate->get(properties[i]);
        if(variant) format.setProperty(properties[i], *variant);
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
        m_textLists.insert(block.document(), QPointer<QTextList>(textList));
    }
}

void KoListStyle::apply(const KoListStyle &other) {
    m_name = other.name();
    m_stylesPrivate->clearAll();
    m_stylesPrivate->copyMissing(other.m_stylesPrivate);
}

bool KoListStyle::operator==(const KoListStyle &other) const {
    kDebug() << "KoListStyle::operator==\n";

    int i=0;
    while(properties[i] != -1) {
        QVariant const *variant = m_stylesPrivate->get(properties[i]);
        if(variant) {
            if(other.m_stylesPrivate->get(properties[i]) != 0)
                return false;
        }
        else {
            QVariant const *otherVariant = m_stylesPrivate->get(properties[i]);
            if(otherVariant == 0 || *otherVariant != *variant)
                return false;
        }
        i++;
    }
    return true;
}

QTextList *KoListStyle::textList(const QTextDocument *doc) {
    return m_textLists[doc];
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
