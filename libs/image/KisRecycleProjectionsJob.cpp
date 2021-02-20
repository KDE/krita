/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRecycleProjectionsJob.h"
#include "KisSafeNodeProjectionStore.h"
#include "kis_paint_device.h"

KisRecycleProjectionsJob::KisRecycleProjectionsJob(KisSafeNodeProjectionStoreBaseWSP projectionStore)
    : m_projectionStore(projectionStore)
{
    setExclusive(true);
}

bool KisRecycleProjectionsJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisRecycleProjectionsJob *otherJob =
        dynamic_cast<const KisRecycleProjectionsJob*>(_otherJob);

    return otherJob &&
        otherJob->m_projectionStore == m_projectionStore;
}

void KisRecycleProjectionsJob::run()
{
    KisSafeNodeProjectionStoreBaseSP store = m_projectionStore;
    if (store) {
        store->recycleProjectionsInSafety();
    }
}

int KisRecycleProjectionsJob::levelOfDetail() const
{
    return 0;
}

QString KisRecycleProjectionsJob::debugName() const
{
    return "KisRecycleProjectionsJob";
}
