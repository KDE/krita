/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisStoragePlugin.h"
#include <QFileInfo>

#include <KisMimeDatabase.h>
#include "KisResourceLoaderRegistry.h"

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

KoResourceSP KisStoragePlugin::resource(const QString &url)
{
    if (!url.contains('/')) return nullptr;

    QStringList parts = url.split('/', QString::SkipEmptyParts);
    if (parts.isEmpty()) return nullptr;

    const QString resourceType = parts[0];
    parts.removeFirst();
    const QString resourceFilename = parts.join('/');

    const QString mime = KisMimeDatabase::mimeTypeForSuffix(resourceFilename);

    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(resourceType, mime);
    if (!loader) {
        qWarning() << "Could not create loader for" << resourceType << resourceFilename << mime;
        return nullptr;
    }

    KoResourceSP resource = loader->create(resourceFilename);
    return loadVersionedResource(resource) ? resource : nullptr;
}

QString KisStoragePlugin::resourceMd5(const QString &url)
{
    // a fallback implementation for the storages with
    // ephemeral resources
    KoResourceSP res = resource(url);
    if (res) {
        return res->md5Sum();
    } else {
        return QString();
    }
}

bool KisStoragePlugin::supportsVersioning() const
{
    return true;
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
