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

#include "KisResourceLoaderRegistry.h"

#include <QApplication>
#include <QString>
#include <QDebug>

#include <KisResourceCacheDb.h>
#include <KisMimeDatabase.h>

KisResourceLoaderRegistry::KisResourceLoaderRegistry(QObject *parent)
    : QObject(parent)
{
}

KisResourceLoaderRegistry::~KisResourceLoaderRegistry()
{
    qDeleteAll(values());
}

KisResourceLoaderRegistry* KisResourceLoaderRegistry::instance()
{
    KisResourceLoaderRegistry *reg = qApp->findChild<KisResourceLoaderRegistry *>(QString());
    if (!reg) {
        reg = new KisResourceLoaderRegistry(qApp);
    }
    return reg;
}

bool KisResourceLoaderRegistry::registerLoader(KisResourceLoaderBase *loader)
{
    add(loader);
    return KisResourceCacheDb::registerResourceType(loader->resourceType());
}

KisResourceLoaderBase *KisResourceLoaderRegistry::loader(const QString &resourceType, const QString &mimetype) const
{
    Q_FOREACH(KisResourceLoaderBase *loader, resourceTypeLoaders(resourceType)) {
        if (loader->mimetypes().contains(mimetype)) return loader;
    }
    return 0;
}

QVector<KisResourceLoaderBase *> KisResourceLoaderRegistry::resourceTypeLoaders(const QString &resourceType) const
{
    QVector<KisResourceLoaderBase *> r;
    Q_FOREACH(KisResourceLoaderBase *loader, values()) {
        if (loader->resourceType() == resourceType) {
            r << loader;
        }
    }
    return r;
}

QStringList KisResourceLoaderRegistry::filters(const QString &resourceType) const
{
    QStringList r;
    Q_FOREACH(KisResourceLoaderBase *loader, resourceTypeLoaders(resourceType)) {
        r.append(loader->filters());
    }
    r.removeDuplicates();
    r.sort();
    return r;
}

QStringList KisResourceLoaderRegistry::mimeTypes(const QString &resourceType) const
{
    QStringList extensions = KisResourceLoaderRegistry::instance()->filters(resourceType);
    QStringList mimeTypes;
    Q_FOREACH(const QString &extension, extensions) {
        mimeTypes << KisMimeDatabase::mimeTypeForSuffix(extension);
    }
    mimeTypes.removeDuplicates();
    mimeTypes.sort();
    return mimeTypes;
}



QStringList KisResourceLoaderRegistry::resourceTypes() const
{
    QStringList r;
    Q_FOREACH(KisResourceLoaderBase *loader, values()) {
        r << loader->resourceType();
    }
    r.removeDuplicates();
    r.sort();

    return r;
}
