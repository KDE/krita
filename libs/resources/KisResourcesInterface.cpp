/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisResourcesInterface.h"


#include <QString>
#include "kis_assert.h"
#include "KisResourcesInterface_p.h"

#include "kis_debug.h"

KisResourcesInterface::KisResourcesInterface()
    : d_ptr(new KisResourcesInterfacePrivate)
{
}

KisResourcesInterface::KisResourcesInterface(KisResourcesInterfacePrivate *dd)
    : d_ptr(dd)
{
}

KisResourcesInterface::~KisResourcesInterface()
{

}

KisResourcesInterface::ResourceSourceAdapter &KisResourcesInterface::source(const QString &type) const
{
    Q_D(const KisResourcesInterface);

    // use double-locking for fetching the source

    ResourceSourceAdapter *source = 0;

    {
        QReadLocker l(&d->lock);
        source = d->findExistingSource(type);
        if (source) return *source;
    }

    {
        QWriteLocker l(&d->lock);
        source = d->findExistingSource(type);
        if (source) return *source;

        source = createSourceImpl(type);

        std::unique_ptr<ResourceSourceAdapter> sourcePtr(source);
        d->sourceAdapters.emplace(type, std::move(sourcePtr));
    }

    KIS_ASSERT(source);
    return *source;
}

KisResourcesInterface::ResourceSourceAdapter::ResourceSourceAdapter()
{
}

KisResourcesInterface::ResourceSourceAdapter::~ResourceSourceAdapter()
{
}

KoResourceSP KisResourcesInterface::ResourceSourceAdapter::bestMatch(const QString md5, const QString filename, const QString name)
{
    QVector<QPair<KoResourceSP, int>> foundResources;

    if (!md5.isEmpty()) {
        Q_FOREACH (KoResourceSP res, resourcesForMD5(md5)) {
            int penalty = 0;

            if (!filename.isEmpty() && filename != res->filename()) {
                /// filename is more important than name, so it gives
                /// higher penalty
                penalty += 2;
            }

            if (!name.isEmpty() && name != res->name()) {
                penalty++;
            }

            foundResources.append(qMakePair(res, penalty));
        }
    } else if (!filename.isEmpty()) {
        Q_FOREACH (KoResourceSP res, resourcesForFilename(filename)) {
            int penalty = 0;

            if (!name.isEmpty() && name != res->name()) {
                penalty++;
            }

            foundResources.append(qMakePair(res, penalty));
        }
    } else if (!name.isEmpty()) {
        Q_FOREACH (KoResourceSP res, resourcesForName(name)) {
            int penalty = 0;
            foundResources.append(qMakePair(res, penalty));
        }
    }

    auto it = std::min_element(foundResources.begin(), foundResources.end(),
                               [] (const QPair<KoResourceSP, int> &lhs,
                               const QPair<KoResourceSP, int> &rhs) {return lhs.second < rhs.second;});

    return it != foundResources.end() ? it->first : KoResourceSP();
}
