/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

QString KisRecalculateGeneratorLayerJob::debugName() const
{
    QString result;
    QDebug dbg(&result);
    dbg << "KisRecalculateGeneratorLayerJob" << m_layer;
    return result;
}
