/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISEMBEDDEDRESOURCESTORAGEPROXY_H
#define KISEMBEDDEDRESOURCESTORAGEPROXY_H

#include "kritaresources_export.h"
#include "KisResourceModel.h"
#include "KisLocalStrokeResources.h"

/**
 * When PSD is loaded into a temporary document, e.g. in a thumbnailer thread,
 * we don't have access to the resources database. Therefore we need to load
 * the embedded resources into a local storage
 */
struct KRITARESOURCES_EXPORT KisEmbeddedResourceStorageProxy {
    KisEmbeddedResourceStorageProxy(const QString &storageLocation);

    void addResource(KoResourceSP resource);;

    /**
     * Returns the resources interface that lets you access all the embedded
     * resources added to the proxy, either before this call, or after it.
     */
    KisResourcesInterfaceSP resourcesInterface();

    /**
     * Return the resources interface that contains a snapshot of all the
     * resources that were added to the proxy by then. The returned interface
     * will not be linked to the proxy anymore.
     *
     * Use this interface for initializing layers styles or paintop preset,
     * which you plan to add to the proxy as well. It will let you avoid
     * cycling shared pointer links (and, therefore, memory leaks).
     *
     * PS:
     * When the document is not temporary and has an officially registered
     * storage, then resourcesInterface() and detachedResourcesInterface()
     * return the same interface, which is KisGlobalResourcesInterface.
     */
    KisResourcesInterfaceSP detachedResourcesInterface();

private:
    QString m_storageLocation;
    QSharedPointer<KisLocalStrokeResources> m_fallbackResourcesInterface;
    KisResourcesInterfaceSP m_rootResourcesInterface;

    KisResourceModel m_stylesModel;
    KisResourceModel m_patternsModel;
    KisResourceModel m_gradientsModel;
};

#endif // KISEMBEDDEDRESOURCESTORAGEPROXY_H
