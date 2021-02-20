/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

void KisResourceLoaderRegistry::registerLoader(KisResourceLoaderBase *loader)
{
    add(loader);
}

KisResourceLoaderBase *KisResourceLoaderRegistry::loader(const QString &resourceType, const QString &mimetype) const
{
    Q_FOREACH(KisResourceLoaderBase *loader, resourceTypeLoaders(resourceType)) {

        if (loader->mimetypes().contains(mimetype)) {
            return loader;
        }
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
