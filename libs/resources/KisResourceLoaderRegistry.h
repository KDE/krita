/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
 * 
 * KisResourceLoaderRegistry has full knowledge of all resource types that are defined for Krita.
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
    void registerLoader(KisResourceLoaderBase* loader);

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
