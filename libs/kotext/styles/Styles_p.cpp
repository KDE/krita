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

#include "Styles_p.h"

#include <QTextFormat>

StylePrivate::StylePrivate() {
}

StylePrivate::~StylePrivate() {
}


void StylePrivate::add(int key, const QVariant &value) {
    m_properties.insert(key, value);
}

void StylePrivate::remove(int key) {
    m_properties.remove(key);
}

const QVariant StylePrivate::value(int key) const {
    return m_properties.value(key);
}

bool StylePrivate::contains(int key) const {
    return m_properties.contains(key);
}

void StylePrivate::copyMissing(const StylePrivate *other) {
    foreach(int key, other->m_properties.keys()) {
        if(! m_properties.contains(key))
            m_properties.insert(key, other->value(key));
    }
}

void StylePrivate::removeDuplicates(const StylePrivate *other) {
    foreach(int key, other->m_properties.keys())
        m_properties.remove(key);
}
