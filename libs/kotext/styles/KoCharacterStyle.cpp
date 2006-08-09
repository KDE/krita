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
#include "KoCharacterStyle.h"

#include "Styles_p.h"

KoCharacterStyle::KoCharacterStyle()
{
    // TODO fill with nice defaults
    m_stylesPrivate = new StylePrivate();
}

KoCharacterStyle::~KoCharacterStyle() {
}

void KoCharacterStyle::setProperty(int key, const QVariant &value) {
    m_stylesPrivate->add(key, value);
}

const QVariant *KoCharacterStyle::get(int key) const {
    return m_stylesPrivate->get(key);
}

double KoCharacterStyle::propertyDouble(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0.0;
    return variant->toDouble();
}

QPen KoCharacterStyle::textOutline () const {
    const QVariant *variant = get(QTextFormat::TextOutline);
    if(variant == 0) {
        QPen pen(Qt::NoPen);
        return pen;
    }
    return qvariant_cast<QPen>(variant);
}

QColor KoCharacterStyle::underlineColor () const {
    const QVariant *variant = get(QTextFormat::TextUnderlineColor);
    if(variant == 0) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(variant);
}

int KoCharacterStyle::propertyInt(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0;
    return variant->toInt();
}

void KoCharacterStyle::applyStyle(QTextCharFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        // TODO
        -1
    };

    int i=0;
    while(properties[i] != -1) {
        format.setProperty(i, get(i));
        i++;
    }
}

#include "KoCharacterStyle.moc"
