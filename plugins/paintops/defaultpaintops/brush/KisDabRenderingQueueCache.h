/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDABRENDERINGQUEUECACHE_H
#define KISDABRENDERINGQUEUECACHE_H

#include "KisDabRenderingQueue.h"
#include "kis_dab_cache_base.h"

#include "kritadefaultpaintops_export.h"

class KisPressureMirrorOption;
class KisPrecisionOption;
class KisPressureSharpnessOption;

class KRITADEFAULTPAINTOPS_EXPORT KisDabRenderingQueueCache : public KisDabRenderingQueue::CacheInterface, public KisDabCacheBase
{
public:

public:
    KisDabRenderingQueueCache();
    ~KisDabRenderingQueueCache();

    void getDabType(bool hasDabInCache,
                    KisDabCacheUtils::DabRenderingResources *resources,
                    const KisDabCacheUtils::DabRequestInfo &request,
                    /* out */
                    KisDabCacheUtils::DabGenerationInfo *di,
                    bool *shouldUseCache) override;

    bool hasSeparateOriginal(KisDabCacheUtils::DabRenderingResources *resources) const override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISDABRENDERINGQUEUECACHE_H
