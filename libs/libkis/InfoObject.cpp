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

#include <kis_properties_configuration.h>

struct InfoObject::Private {
    Private() {}

    KisPropertiesConfigurationSP properties;
};

InfoObject::InfoObject(KisPropertiesConfigurationSP configuration)
    : QObject(0)
    , d(new Private)
{
    d->properties = configuration;
}

InfoObject::InfoObject(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->properties = new KisPropertiesConfiguration();
}

InfoObject::~InfoObject()
{
    delete d;
}

QMap<QString, QVariant> InfoObject::properties() const
{
    return d->properties->getProperties();
}

void InfoObject::setProperties(QMap<QString, QVariant> proprertyMap)
{
    Q_FOREACH(const QString & key, proprertyMap.keys()) {
        d->properties->setProperty(key, proprertyMap[key]);
    }
}

void InfoObject::setProperty(const QString &key, QVariant value)
{
    d->properties->setProperty(key, value);
}

QVariant InfoObject::property(const QString &key)
{
    QVariant v;
    if (d->properties->hasProperty(key)) {
        d->properties->getProperty(key, v);
    }
    return v;
}

KisPropertiesConfigurationSP InfoObject::configuration() const
{
    return d->properties;
}


