/*
   Copyright (c) 2006, 2011 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>
   Copyright (c) 2011 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#include "KoResourceManager_p.h"

#include <QVariant>
#include <kundo2stack.h>
#include <kdebug.h>

#include "KoShape.h"

void KoResourceManager::setResource(int key, const QVariant &value)
{
    if (m_resources.contains(key)) {
        if (m_resources.value(key) == value)
            return;
        m_resources[key] = value;
    } else {
        m_resources.insert(key, value);
    }
}

QVariant KoResourceManager::resource(int key) const
{
    if (!m_resources.contains(key)) {
        QVariant empty;
        return empty;
    } else
        return m_resources.value(key);
}

void KoResourceManager::setResource(int key, const KoColor &color)
{
    QVariant v;
    v.setValue(color);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, const KoUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

KoColor KoResourceManager::koColorResource(int key) const
{
    if (! m_resources.contains(key)) {
        KoColor empty;
        return empty;
    }
    return resource(key).value<KoColor>();
}

KoShape *KoResourceManager::koShapeResource(int key) const
{
    if (! m_resources.contains(key))
        return 0;

    return resource(key).value<KoShape *>();
}


KoUnit KoResourceManager::unitResource(int key) const
{
    return resource(key).value<KoUnit>();
}

bool KoResourceManager::boolResource(int key) const
{
    if (! m_resources.contains(key))
        return false;
    return m_resources[key].toBool();
}

int KoResourceManager::intResource(int key) const
{
    if (! m_resources.contains(key))
        return 0;
    return m_resources[key].toInt();
}

QString KoResourceManager::stringResource(int key) const
{
    if (! m_resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

QSizeF KoResourceManager::sizeResource(int key) const
{
    if (! m_resources.contains(key)) {
        QSizeF empty;
        return empty;
    }
    return qvariant_cast<QSizeF>(resource(key));
}

bool KoResourceManager::hasResource(int key) const
{
    return m_resources.contains(key);
}

void KoResourceManager::clearResource(int key)
{
    if (! m_resources.contains(key))
        return;
    m_resources.remove(key);
    QVariant empty;
}
