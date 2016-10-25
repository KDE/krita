/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "InfoObject.h"

struct InfoObject::Private {
    Private() {}
};

InfoObject::InfoObject(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
}

InfoObject::~InfoObject() 
{
    delete d;
}

QMap<QString, QVariant> InfoObject::properties() const
{
    return QMap<QString, QVariant>();
}

void InfoObject::setproperties(QMap<QString, QVariant> value)
{
}




void InfoObject::setProperty(const QString &key, QVariant value)
{
}

QVariant InfoObject::property(const QString &key)
{
    return QVariant();
}



