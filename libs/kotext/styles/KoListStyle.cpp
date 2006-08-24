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

#include "KoListStyle.h"
#include "Styles_p.h"
#include "KoTextBlockData.h"

#include <QTextBlock>
#include <QTextCursor>
#include <kdebug.h>

KoListStyle::KoListStyle() {
    m_stylesPrivate = new StylePrivate();
    setStyle(DiscItem);
    setStartValue(1);
}

KoListStyle::KoListStyle(const KoListStyle &orig) {
    m_stylesPrivate = new StylePrivate();
    m_stylesPrivate->copyMissing(orig.m_stylesPrivate);
    m_name = orig.name();
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

void KoListStyle::applyStyle(QTextBlock &block) {
    QTextList *textList = m_textLists.value(block.document());
    if(textList && block.textList() && block.textList() != textList) // remove old one
        block.textList()->remove(block);
    if(block.textList() == 0 && textList) // add if new
        textList->add(block);

    QTextListFormat format;
    if(block.textList())
        format = block.textList()->format();

    // copy all relevant properties.
    static const int properties[] = {
        QTextListFormat::ListStyle,
        ListItemPrefix,
        ListItemSuffix,
        ConsecutiveNumbering,
        StartValue,
        Level,
        DisplayLevel,
        CharacterStyleId,
        BulletCharacter,
        BulletSize,
        -1
    };

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
