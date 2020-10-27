/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisStoragePlugin.h"
#include <QFileInfo>

class KisStoragePlugin::Private
{
public:
    QString location;
    QDateTime timestamp;
};

KisStoragePlugin::KisStoragePlugin(const QString &location)
    : d(new Private())
{
    d->location = location;

    if (!QFileInfo(d->location).exists()) {
        d->timestamp = QDateTime::currentDateTime();
    }
}

KisStoragePlugin::~KisStoragePlugin()
{
}

QDateTime KisStoragePlugin::timestamp()
{
    if (d->timestamp.isNull()) {
        return QFileInfo(d->location).lastModified();
    }
    return d->timestamp;
}

QString KisStoragePlugin::location() const
{
    return d->location;
}
