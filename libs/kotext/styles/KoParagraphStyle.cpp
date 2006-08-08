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
#include "KoParagraphStyle.h"

#include "Styles_p.h"

KoParagraphStyle::KoParagraphStyle()
    : m_charStyle(0),
    m_parent(0),
    m_next(0)
{
    m_stylesPrivate = new StylePrivate();
    // TODO fill with nice defaults.
}

KoParagraphStyle::~KoParagraphStyle() {
    delete m_stylesPrivate;
    m_stylesPrivate = 0;
}

void KoParagraphStyle::setParent(KoParagraphStyle *parent) {
    Q_ASSERT(parent != this);
    if(m_parent)
        m_stylesPrivate->copyMissing(m_parent->m_stylesPrivate);
    m_parent = parent;
    if(m_parent)
        m_stylesPrivate->removeDuplicates(m_parent->m_stylesPrivate);
}

void KoParagraphStyle::setProperty(int key, const QVariant &value) {
    if(m_parent) {
        QVariant const *var = m_parent->get(key);
        if(var && (*var) == value) { // same as parent, so its actually a reset.
            m_stylesPrivate->remove(key);
            return;
        }
    }
    m_stylesPrivate->add(key, value);
}

const QVariant *KoParagraphStyle::get(int key) const {
    QVariant const *var = m_stylesPrivate->get(key);
    if(var == 0 && m_parent)
        var = m_parent->get(key);
    return var;
}

double KoParagraphStyle::propertyDouble(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0.0;
    return variant->toDouble();
}

int KoParagraphStyle::propertyInt(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0;
    return variant->toInt();
}

