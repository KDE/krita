/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISDABRENDERINGQUEUE_H
#define KISDABRENDERINGQUEUE_H

#include <QScopedPointer>

#include "kritadefaultpaintops_export.h"

#include <QList>
class KisDabRenderingJob;
struct KisRenderedDab;

#include "KisDabCacheUtils.h"

class KRITADEFAULTPAINTOPS_EXPORT KisDabRenderingQueue
{
public:
    struct CacheInterface {
        virtual ~CacheInterface() {}
        virtual void getDabType(bool hasDabInCache,
                                KisDabCacheUtils::DabRenderingResources *resources,
                                const KisDabCacheUtils::DabRequestInfo &request,
                                /* out */
                                KisDabCacheUtils::DabGenerationInfo *di,
                                bool *shouldUseCache) = 0;

        virtual bool hasSeparateOriginal(KisDabCacheUtils::DabRenderingResources *resources) const = 0;
    };


public:
    KisDabRenderingQueue(const KoColorSpace *cs, KisDabCacheUtils::ResourcesFactory resourcesFactory);
    ~KisDabRenderingQueue();

    KisDabRenderingJobSP addDab(const KisDabCacheUtils::DabRequestInfo &request,
                               qreal opacity, qreal flow);

    QList<KisDabRenderingJobSP> notifyJobFinished(int seqNo, int usecsTime = -1);

    QList<KisRenderedDab> takeReadyDabs(bool returnMutableDabs = false, int oneTimeLimit = -1, bool *someDabsLeft = 0);

    bool hasPreparedDabs() const;

    void setCacheInterface(CacheInterface *interface);

    KisFixedPaintDeviceSP fetchCachedPaintDevce();

    void putResourcesToCache(KisDabCacheUtils::DabRenderingResources *resources);
    KisDabCacheUtils::DabRenderingResources* fetchResourcesFromCache();

    qreal averageExecutionTime() const;
    int averageDabSize() const;

    int testingGetQueueSize() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISDABRENDERINGQUEUE_H
