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

KoListStyle::KoListStyle() {
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

const QString &KoListStyle::listItemPrefix() const {
    const QVariant *variant = m_stylesPrivate->get(ListItemPrefix);
    if(variant == 0) {
        QString string;
        return string;
    }
    return qvariant_cast<QString>(variant);
}

const QString &KoListStyle::listItemSuffix() const {
    const QVariant *variant = m_stylesPrivate->get(ListItemSuffix);
    if(variant == 0) {
        QString string;
        return string;
    }
    return qvariant_cast<QString>(variant);
}

void KoListStyle::applyStyle(QTextBlockFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        ListStyle,
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
}
