/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISRECYCLEPROJECTIONSJOB_H
#define KISRECYCLEPROJECTIONSJOB_H

#include "kis_types.h"
#include "kis_spontaneous_job.h"

class QMutex;
class KisSafeNodeProjectionStoreBase;
typedef KisWeakSharedPtr<KisSafeNodeProjectionStoreBase> KisSafeNodeProjectionStoreBaseWSP;

/**
 * This is a simple job for initiating recycling of KisLayer's
 * projections in an exclusive context. The problem is that the
 * projection might still be used by update workers after it
 * has been disabled. So this cleaning should run in an exclusive
 * context.
 */
class KRITAIMAGE_EXPORT KisRecycleProjectionsJob : public KisSpontaneousJob
{
public:
    KisRecycleProjectionsJob(KisSafeNodeProjectionStoreBaseWSP projectionStore);

    bool overrides(const KisSpontaneousJob *otherJob) override;
    void run() override;
    int levelOfDetail() const override;

    QString debugName() const override;

private:
    KisSafeNodeProjectionStoreBaseWSP m_projectionStore;
};

#endif // KISRECYCLEPROJECTIONSJOB_H
