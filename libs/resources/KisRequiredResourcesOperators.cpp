/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRequiredResourcesOperators.h"

#include <QApplication>
#include <QBuffer>
#include <QThread>
#include <KisLocalStrokeResources.h>
#include <KisResourceLoaderRegistry.h>
#include <KisMimeDatabase.h>


bool KisRequiredResourcesOperators::detail::isLocalResourcesStorage(KisResourcesInterfaceSP resourcesInterface)
{
    return resourcesInterface.dynamicCast<KisLocalStrokeResources>();
}

void KisRequiredResourcesOperators::detail::assertInGuiThread()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(QThread::currentThread() == qApp->thread());
}

KisResourcesInterfaceSP KisRequiredResourcesOperators::detail::createLocalResourcesStorage(const QList<KoResourceSP> &resources)
{
    return QSharedPointer<KisLocalStrokeResources>::create(resources);
}

void KisRequiredResourcesOperators::detail::addResourceOrWarnIfNotLoaded(KoResourceLoadResult loadedResource, QList<KoResourceSP> *resources, KisResourcesInterfaceSP resourcesInterface)
{
    switch (loadedResource.type()) {
    case KoResourceLoadResult::ExistingResource:
        *resources << loadedResource.resource();
        break;
    case KoResourceLoadResult::EmbeddedResource: {
        /**
         * Some resources, like filter configurations may be assigned to the
         * layers without being loaded to the resource system. In such a case,
         * the embedded resources will be loaded here, when we make a snapshot.
         */

        const KoEmbeddedResource embeddedResource = loadedResource.embeddedResource();
        const KoResourceSignature sig = embeddedResource.signature();

        KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(sig.type, KisMimeDatabase::mimeTypeForFile(sig.filename));

        if (!loader) {
            qWarning() << "createLocalResourcesSnapshot: Could not create a loader for resource" << sig;
            return;
        }

        QByteArray ba = embeddedResource.data();
        QBuffer buf(&ba);
        buf.open(QBuffer::ReadOnly);

        KoResourceSP resource = loader->load(sig.filename, buf, resourcesInterface);
        resource->setMD5Sum(sig.md5);
        resource->setVersion(0);
        resource->setDirty(false);

        if (resource) {
            *resources << resource;
        } else {
            qWarning() << "createLocalResourcesSnapshot: Could not import embedded resource" << sig;
        }
        break;
    }
    case KoResourceLoadResult::FailedLink:
        qWarning() << "createLocalResourcesSnapshot: failed to load a linked resource:" << loadedResource.signature();
        break;
    }
}
