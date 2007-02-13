/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextEditingFactory.h"

class KoTextEditingFactory::Private {
public:
    Private(const QString &identifier)
        : id(identifier)
    {
    }

    const QString id;
};

KoTextEditingFactory::KoTextEditingFactory(QObject *parent, const QString &id)
    : QObject(parent)
    , d( new Private(id) )
{
}

KoTextEditingFactory::~KoTextEditingFactory() {
    delete d;
}

const KoID KoTextEditingFactory::id() const {
    // can we please get rid of KoID.  There is no reason that we should encode a name with every Id.
    return KoID(d->id, "");
}

const QString &KoTextEditingFactory::objectId() const {
    return d->id;
}

#include "KoTextEditingFactory.moc"
