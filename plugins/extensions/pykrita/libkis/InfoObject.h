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
#ifndef LIBKIS_INFOOBJECT_H
#define LIBKIS_INFOOBJECT_H

#include <QObject>
#include <kis_properties_configuration.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * InfoObject
 */
class KRITALIBKIS_EXPORT InfoObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InfoObject)

    Q_PROPERTY(QMap<QString, QVariant> properties READ properties WRITE setproperties)

public:
    explicit InfoObject(QObject *parent = 0);
    virtual ~InfoObject();

    QMap<QString, QVariant> properties() const;
    void setproperties(QMap<QString, QVariant> proprertyMap);

public Q_SLOTS:

    void setProperty(const QString &key, QVariant value);
    QVariant property(const QString &key);


private:
    struct Private;
    Private *d;

};

#endif // LIBKIS_INFOOBJECT_H
