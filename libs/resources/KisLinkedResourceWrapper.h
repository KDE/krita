/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISLINKEDRESOURCEWRAPPER_H
#define KISLINKEDRESOURCEWRAPPER_H

#include <kis_assert.h>
#include <KoResource.h>
#include <KisResourcesInterface.h>

template<class ResourceType>
class KisLinkedResourceWrapper
{
public:
    using ResourceTypeSP = QSharedPointer<ResourceType>;

    KisLinkedResourceWrapper()
    {
    }

    KisLinkedResourceWrapper(ResourceTypeSP resource)
    {
        if (!resource) return;

        m_type = resource->resourceType().first;
        m_md5 = resource->md5Sum();
        m_filename = resource->filename();
        m_name = resource->name();
    }

    KisLinkedResourceWrapper(const QString &md5, const QString &filename, const QString &name)
        : m_md5(md5)
        , m_filename(filename)
        , m_name(name)
    {
    }


    ResourceTypeSP resource(KisResourcesInterfaceSP resourcesInterface) const
    {
        ResourceTypeSP result;
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_type.isEmpty(), result);

        auto source = resourcesInterface->source<ResourceType>(m_type);

        return source.resource(m_md5, m_filename, m_name);
    }

    bool isValid() const {
        return !m_type.isEmpty();
    }

private:
    QString m_type;
    QString m_md5;
    QString m_filename;
    QString m_name;
};

#endif // KISLINKEDRESOURCEWRAPPER_H
