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

//#define SANITY_CHECKS


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

KisResourcesInterface::ResourceSourceAdapter::ResourceSourceAdapter(const QString &type)
    : m_type(type)
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

            if (!res->active()) {
                penalty += 100;
            }

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
    }

#ifdef SANITY_CHECKS
    /**
     * When we request a resource using MD5, but it is found only via its
     * filename, it is, most probably, a problem with the preset. Namely,
     * the MD5 tag saved into the preset is wrong. Let's just warn about
     * that.
     */
    const bool warnAboutIncorrectMd5Fetch =
        foundResources.isEmpty() && !md5.isEmpty();
#endif

    if (foundResources.isEmpty()) {
        if (!filename.isEmpty()) {
            Q_FOREACH (KoResourceSP res, resourcesForFilename(filename)) {
                int penalty = 0;

                if (!res->active()) {
                    penalty += 100;
                }

                if (!name.isEmpty() && name != res->name()) {
                    penalty++;
                }

                foundResources.append(qMakePair(res, penalty));
            }
        } else if (!name.isEmpty()) {
            Q_FOREACH (KoResourceSP res, resourcesForName(name)) {
                int penalty = 0;

                if (!res->active()) {
                    penalty += 100;
                }

                foundResources.append(qMakePair(res, penalty));
            }
        }
    }

    auto it = std::min_element(foundResources.begin(), foundResources.end(),
                               [] (const QPair<KoResourceSP, int> &lhs,
                               const QPair<KoResourceSP, int> &rhs) {return lhs.second < rhs.second;});

    KoResourceSP result = it != foundResources.end() ? it->first : KoResourceSP();

#ifdef SANITY_CHECKS
    if (warnAboutIncorrectMd5Fetch && result) {
        qWarning() << "KisResourcesInterface::ResourceSourceAdapter::bestMatch: failed to fetch a resource using md5; falling back for filename...";
        qWarning() << "    requested:" << ppVar(md5) << ppVar(filename) << ppVar(name);
        qWarning() << "    found:" << result;
    }
#endif

    return result;
}

KoResourceLoadResult KisResourcesInterface::ResourceSourceAdapter::bestMatchLoadResult(const QString md5, const QString filename, const QString name)
{
    KoResourceSP resource = bestMatch(md5, filename, name);
    return
        resource ?
        KoResourceLoadResult(resource) :
        KoResourceLoadResult(KoResourceSignature(m_type, md5, filename, name));
}
