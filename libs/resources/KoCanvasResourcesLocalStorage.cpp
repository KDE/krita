/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KoCanvasResourcesLocalStorage.h"

#include <QVariant>
#include <QMap>


struct KoCanvasResourcesLocalStorage::Private
{
    QMap<int, QVariant> resources;
};

KoCanvasResourcesLocalStorage::KoCanvasResourcesLocalStorage()
    : m_d(new Private)
{
}

KoCanvasResourcesLocalStorage::KoCanvasResourcesLocalStorage(const KoCanvasResourcesLocalStorage &rhs)
    : m_d(new Private)
{
    m_d->resources = rhs.m_d->resources;
}

KoCanvasResourcesLocalStorage &KoCanvasResourcesLocalStorage::operator=(const KoCanvasResourcesLocalStorage &rhs)
{
    m_d->resources = rhs.m_d->resources;
    return *this;
}

KoCanvasResourcesLocalStorage::~KoCanvasResourcesLocalStorage()
{
}


QVariant KoCanvasResourcesLocalStorage::resource(int key) const
{
    return m_d->resources.value(key, QVariant());
}

void KoCanvasResourcesLocalStorage::storeResource(int key, const QVariant &resource)
{
    m_d->resources[key] = resource;
}
