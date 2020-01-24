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

#ifndef KISRESOURCELOADERREGISTRY_H
#define KISRESOURCELOADERREGISTRY_H

#include <QObject>
#include <QSet>
#include <QStringList>

#include <KoGenericRegistry.h>
#include "KisResourceLoader.h"

#include <kritaresources_export.h>

/**
 * @brief The KisResourceLoaderRegistry class manages the loader plugins for resources. Every resource can be loaded
 * by a KisResourceLoader instance. A loader corresponds to a particular file type. Resources are organized in
 * folders that represent the main type of a certain resource (brushes) and subtypes, that identify a particular
 * resource format (gbr, gih, png, svg).
 */
class KRITARESOURCES_EXPORT KisResourceLoaderRegistry : public QObject, public KoGenericRegistry<KisResourceLoaderBase*>
{
    Q_OBJECT
public:
    ~KisResourceLoaderRegistry() override;

    static KisResourceLoaderRegistry *instance();

    /**
     * Adds the given loader and registers its type in the database, if it hasn't been registered yet.
     */
    bool registerLoader(KisResourceLoaderBase* loader);

    /// @return the first loader for the given resource type and mimetype
    KisResourceLoaderBase *loader(const QString &resourceType, const QString &mimetype) const;

    /**
     * @return a list of filename extensions that can be present for the given resource type
     */
    QStringList filters(const QString &resourceType) const;

    /**
     * @return a list of mimetypes that can be loaded for the given resourde type
     */
    QStringList mimeTypes(const QString &resourceType) const;

    /**
     * @return the list of folders for which resource loaders have been registered
     */
    QStringList resourceTypes() const;

    /**
     * @return a list of loader plugins that can handle the resources stored in the folder. A folder can contain multiple subtypes.
     */
    QVector<KisResourceLoaderBase*> resourceTypeLoaders(const QString &resourceType) const;

private:

    KisResourceLoaderRegistry(QObject *parent);
    KisResourceLoaderRegistry(const KisResourceLoaderRegistry&);
    KisResourceLoaderRegistry operator=(const KisResourceLoaderRegistry&);
};

#endif // KISRESOURCELOADERREGISTRY_H
