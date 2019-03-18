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
#include "KisRecycleProjectionsJob.h"

#include "kis_layer.h"


KisRecycleProjectionsJob::KisRecycleProjectionsJob(KisLayerWSP layer)
    : m_layer(layer)
{
    setExclusive(true);
}

bool KisRecycleProjectionsJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisRecycleProjectionsJob *otherJob =
        dynamic_cast<const KisRecycleProjectionsJob*>(_otherJob);

    return otherJob &&
        otherJob->m_layer == m_layer;
}

void KisRecycleProjectionsJob::run()
{
    KisLayerSP layer = m_layer;
    if (layer) {
        layer->recycleProjectionsInSafety();
    }
}

int KisRecycleProjectionsJob::levelOfDetail() const
{
    return 0;
}
