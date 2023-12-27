/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisEmbeddedResourceStorageProxy.h"

#include "kis_assert.h"
#include "KisGlobalResourcesInterface.h"

KisEmbeddedResourceStorageProxy::KisEmbeddedResourceStorageProxy(const QString &storageLocation)
    : m_storageLocation(storageLocation),
      m_stylesModel(ResourceType::LayerStyles),
      m_patternsModel(ResourceType::Patterns),
      m_gradientsModel(ResourceType::Gradients)
{
    if (m_storageLocation.isEmpty()) {
        m_fallbackResourcesInterface.reset(new KisLocalStrokeResources());
        m_rootResourcesInterface = m_fallbackResourcesInterface;
    } else {
        m_rootResourcesInterface = KisGlobalResourcesInterface::instance();
    }
}

void KisEmbeddedResourceStorageProxy::addResource(KoResourceSP resource)
{
    if (m_fallbackResourcesInterface) {
        if (resource->resourceType().first == ResourceType::LayerStyles) {
            resource = resource->clone();
        } else {
            m_fallbackResourcesInterface->addResource(resource);
        }
    } else {
        if (resource->resourceType().first == ResourceType::LayerStyles) {
            m_stylesModel.addResource(resource, m_storageLocation);
        } else if (resource->resourceType().first == ResourceType::Patterns) {
            m_patternsModel.addResource(resource, m_storageLocation);
        } else if (resource->resourceType().first == ResourceType::Gradients) {
            m_gradientsModel.addResource(resource, m_storageLocation);
        } else {
            qWarning() << "WARNING: fallback resource proxy is not implemented for type" << resource->resourceType().first;
            KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "fallback resource proxy is not implemented");
        }
    }
}

KisResourcesInterfaceSP KisEmbeddedResourceStorageProxy::resourcesInterface()
{
    return m_rootResourcesInterface;
}

KisResourcesInterfaceSP KisEmbeddedResourceStorageProxy::detachedResourcesInterface()
{
    return m_fallbackResourcesInterface ?
        KisResourcesInterfaceSP(m_fallbackResourcesInterface->clone()) :
        m_rootResourcesInterface;
}
