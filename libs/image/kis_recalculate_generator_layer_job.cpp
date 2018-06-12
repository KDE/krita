/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_recalculate_generator_layer_job.h"

#include "generator/kis_generator_layer.h"
#include "kis_debug.h"
#include "kis_layer.h"
#include "kis_image.h"


KisRecalculateGeneratorLayerJob::KisRecalculateGeneratorLayerJob(KisGeneratorLayerSP layer)
    : m_layer(layer)
{
    setExclusive(true);
}

bool KisRecalculateGeneratorLayerJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisRecalculateGeneratorLayerJob *otherJob =
        dynamic_cast<const KisRecalculateGeneratorLayerJob*>(_otherJob);

    return otherJob && otherJob->m_layer == m_layer;
}

void KisRecalculateGeneratorLayerJob::run()
{
    /**
     * The layer might have been deleted from the layers stack. In
     * such a case, don't try do update it.
     */
    if (!m_layer->parent()) return;

    m_layer->update();
}

int KisRecalculateGeneratorLayerJob::levelOfDetail() const
{
    return 0;
}
