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

#include "KoInlineObject.h"


class InlineObjectPrivate {
public:
    InlineObjectPrivate()
        : manager(0),
        id(-1),
        propertyChangeListener(0)
    {
    }

    KoInlineTextObjectManager *manager;
    int id;
    bool propertyChangeListener;
};

KoInlineObject::KoInlineObject(bool propertyChangeListener)
    : d( new InlineObjectPrivate )
{
    d->propertyChangeListener = propertyChangeListener;
}

KoInlineObject::~KoInlineObject() {
    delete d;
}

void KoInlineObject::setManager(KoInlineTextObjectManager *manager) {
    d->manager = manager;
}

KoInlineTextObjectManager *KoInlineObject::manager() {
    return d->manager;
}

void KoInlineObject::propertyChanged(Property key, const QVariant &value) {
    Q_UNUSED(key);
    Q_UNUSED(value);
}

int KoInlineObject::id() const {
    return d->id;
}
void KoInlineObject::setId(int id) {
    d->id = id;
}

bool KoInlineObject::propertyChangeListener() const {
    return d->propertyChangeListener;
}
