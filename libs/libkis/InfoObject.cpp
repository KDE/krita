/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

bool InfoObject::operator==(const InfoObject &other) const
{
    return (d->properties == other.d->properties);
}

bool InfoObject::operator!=(const InfoObject &other) const
{
    return !(operator==(other));
}

QMap<QString, QVariant> InfoObject::properties() const
{
    QMap<QString, QVariant> map = d->properties->getProperties();

    for (const QString &key : map.keys()) {
        QVariant v = map.value(key);

        if (v.isValid() && v.type() == QVariant::UserType && v.userType() == qMetaTypeId<KoColor>()) {
            map[key] = QVariant::fromValue(v.value<KoColor>().toXML());
        }
    }

    return map;
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

        if (v.isValid() && v.type() == QVariant::UserType && v.userType() == qMetaTypeId<KoColor>()) {
            return QVariant::fromValue(v.value<KoColor>().toXML());
        }
    }

    return v;
}

KisPropertiesConfigurationSP InfoObject::configuration() const
{
    return d->properties;
}


