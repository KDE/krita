/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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



#include "KoFindMatch.h"

#include <QVariant>

class Q_DECL_HIDDEN KoFindMatch::Private : public QSharedData
{
public:
    Private() { }
    ~Private() { }
    Private(const Private &other)
            : QSharedData(other),
            container(other.container),
            location(other.location)
    { }

    QVariant container;
    QVariant location;
};

KoFindMatch::KoFindMatch()
    : d(new Private)
{
}

KoFindMatch::KoFindMatch(const QVariant &container, const QVariant &location)
    : d(new Private)
{
    d->container = container;
    d->location = location;
}

KoFindMatch::KoFindMatch(const KoFindMatch &other)
    : d(other.d)
{
}

KoFindMatch::~KoFindMatch()
{
}

KoFindMatch &KoFindMatch::operator=(const KoFindMatch &other)
{
    d = other.d;
    return *this;
}

bool KoFindMatch::operator==(const KoFindMatch &other) const
{
    return d->container == other.d->container && d->location == other.d->location;
}

bool KoFindMatch::isValid() const
{
    return d->container.isValid() && d->location.isValid();
}

QVariant KoFindMatch::container() const
{
    return d->container;
}

void KoFindMatch::setContainer(const QVariant &container)
{
    d->container = container;
}

QVariant KoFindMatch::location() const
{
    return d->location;
}

void KoFindMatch::setLocation(const QVariant &location)
{
    d->location = location;
}

