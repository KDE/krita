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

#include "Styles_p.h"

StylePrivate::StylePrivate() {
}

StylePrivate::~StylePrivate() {
}


void StylePrivate::add(int key, const QVariant &value) {
    for(int i=m_properties.count()-1; i >= 0; i--) {
        if(m_properties[i].key == key) {
            m_properties[i].value = value;
            return;
        }
    }
    Property prop(key, value);
    m_properties.append(prop);
}

void StylePrivate::remove(int key) {
    for(int i=m_properties.count()-1; i >= 0; i--) {
        if(m_properties[i].key == key) {
            m_properties.remove(i);
            return;
        }
    }
}

const QVariant *StylePrivate::get(int key) const {
    for(int i=m_properties.count()-1; i >= 0; i--) {
        if(m_properties[i].key == key) {
            return &m_properties[i].value;
        }
    }
    return 0;
}

bool StylePrivate::contains(int key) const {
    for(int i=m_properties.count()-1; i >= 0; i--) {
        if(m_properties[i].key == key) {
            return true;
        }
    }
    return false;
}

void StylePrivate::copyMissing(const StylePrivate *other) {
    for(int i=other->m_properties.count()-1; i >= 0; i--) {
        bool found = false;
        for(int x=m_properties.count()-1; x >= 0; x--) {
            if(other->m_properties[i].key == m_properties[x].key) {
                found = true;
                break;
            }
        }
        if(!found) // we don't have that key. Copy it.
            m_properties.append( other->m_properties[i] );
    }
}

void StylePrivate::removeDuplicates(const StylePrivate *other) {
    for(int i=other->m_properties.count()-1; i >= 0; i--) {
        for(int x=m_properties.count()-1; x >= 0; x--) {
            if(other->m_properties[i] == m_properties[x]) {
                m_properties.remove(x);
                break;
            }
        }
    }
}
