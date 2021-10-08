/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLocalStrokeResources.h"
#include "KisResourcesInterface_p.h"

#include <QFileInfo>
#include "kis_assert.h"
#include "kis_debug.h"

namespace {
class LocalResourcesSource : public KisResourcesInterface::ResourceSourceAdapter
{
public:
    LocalResourcesSource(const QString &resourceType, const QList<KoResourceSP> &cachedResources)
        : m_resourceType(resourceType)
        , m_cachedResources(cachedResources)
    {
    }
protected:
    QVector<KoResourceSP> resourcesForFilename(const QString &filename) const override {
        QVector<KoResourceSP> resources;
        Q_FOREACH(KoResourceSP res, m_cachedResources) {
            if (res->filename() == filename && res->resourceType().first == m_resourceType) {
                resources << res;
            }
        }
        return resources;
    }

    QVector<KoResourceSP> resourcesForName(const QString &name) const override {
        QVector<KoResourceSP> resources;
        Q_FOREACH(KoResourceSP res, m_cachedResources) {
            if (res->name() == name && res->resourceType().first == m_resourceType) {
                resources << res;
            }
        }
        return resources;
    }

    QVector<KoResourceSP> resourcesForMD5(const QString &md5) const override {
        QVector<KoResourceSP> resources;
        Q_FOREACH(KoResourceSP res, m_cachedResources) {
            if (res->md5Sum() == md5 && res->resourceType().first == m_resourceType) {
                resources << res;
            }
        }
        return resources;
    }

public:

    KoResourceSP fallbackResource() const override {
        auto it = std::find_if(m_cachedResources.begin(),
                               m_cachedResources.end(),
                               [this] (KoResourceSP res) {
                return res->resourceType().first == this->m_resourceType;
    });
        return it != m_cachedResources.end() ? *it : KoResourceSP();
    }

private:
    const QString m_resourceType;
    const QList<KoResourceSP> &m_cachedResources;
};
}

class KisLocalStrokeResourcesPrivate : public KisResourcesInterfacePrivate
{
public:
    KisLocalStrokeResourcesPrivate(const QList<KoResourceSP> &_localResources)
        : localResources(_localResources)
    {

        // sanity check that we don't have any null resources
        KIS_SAFE_ASSERT_RECOVER(!localResources.contains(KoResourceSP())) {
            localResources.removeAll(KoResourceSP());
        }


    }
    QList<KoResourceSP> localResources;
};


KisLocalStrokeResources::KisLocalStrokeResources(const QList<KoResourceSP> &localResources)
    : KisResourcesInterface(new KisLocalStrokeResourcesPrivate(localResources))
{
}

void KisLocalStrokeResources::addResource(KoResourceSP resource)
{
    Q_D(KisLocalStrokeResources);
    d->localResources.append(resource);
}

KisResourcesInterface::ResourceSourceAdapter *KisLocalStrokeResources::createSourceImpl(const QString &type) const
{
    Q_D(const KisLocalStrokeResources);
    return new LocalResourcesSource(type, d->localResources);
}
