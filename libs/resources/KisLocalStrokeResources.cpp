/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLocalStrokeResources.h"
#include "KisResourcesInterface_p.h"

#include <QFileInfo>
#include "kis_assert.h"

namespace {
class LocalResourcesSource : public KisResourcesInterface::ResourceSourceAdapter
{
public:
    LocalResourcesSource(const QString &resourceType, const QList<KoResourceSP> &cachedResources)
        : m_resourceType(resourceType),
          m_cachedResources(cachedResources)
    {
    }
protected:
    KoResourceSP resourceForFilename(const QString& filename) const override {
        auto it = std::find_if(m_cachedResources.begin(),
                               m_cachedResources.end(),
                               [this, filename] (KoResourceSP res) {
                                   return res->resourceType().first == this->m_resourceType &&
                                       (res->filename() == filename ||
                                        QFileInfo(res->filename()).fileName() == filename);
                               });
        return it != m_cachedResources.end() ? *it : KoResourceSP();
    }

    KoResourceSP resourceForName(const QString& name) const override {
        qDebug() << "LocalResourceSource::resourceForName" << name;
        auto it = std::find_if(m_cachedResources.begin(),
                               m_cachedResources.end(),
                               [this, name] (KoResourceSP res) {
                                   return res->resourceType().first == this->m_resourceType &&
                                       res->name() == name;
                               });
        return it != m_cachedResources.end() ? *it : KoResourceSP();
    }

    KoResourceSP resourceForMD5(const QByteArray& md5) const override {
        auto it = std::find_if(m_cachedResources.begin(),
                               m_cachedResources.end(),
                               [this, md5] (KoResourceSP res) {
                                   return res->resourceType().first == this->m_resourceType &&
                                       res->md5() == md5;
                               });
        return it != m_cachedResources.end() ? *it : KoResourceSP();
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
