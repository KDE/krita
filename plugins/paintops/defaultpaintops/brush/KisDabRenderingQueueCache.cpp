/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDabRenderingQueueCache.h"

struct KisDabRenderingQueueCache::Private
{
    Private()
    {
    }
};

KisDabRenderingQueueCache::KisDabRenderingQueueCache()
    : m_d(new Private())
{
}

KisDabRenderingQueueCache::~KisDabRenderingQueueCache()
{
}

void KisDabRenderingQueueCache::getDabType(bool hasDabInCache, KisDabCacheUtils::DabRenderingResources *resources, const KisDabCacheUtils::DabRequestInfo &request, KisDabCacheUtils::DabGenerationInfo *di, bool *shouldUseCache)
{
    fetchDabGenerationInfo(hasDabInCache, resources, request, di, shouldUseCache);
}

bool KisDabRenderingQueueCache::hasSeparateOriginal(KisDabCacheUtils::DabRenderingResources *resources) const
{
    return needSeparateOriginal(resources->textureOption.data(), resources->sharpnessOption.data());
}
