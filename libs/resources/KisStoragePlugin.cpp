/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
