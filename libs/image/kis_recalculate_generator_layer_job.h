/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_RECALCULATE_GENERATOR_LAYER_JOB_H
#define __KIS_RECALCULATE_GENERATOR_LAYER_JOB_H

#include "kis_types.h"
#include "kis_spontaneous_job.h"


class KRITAIMAGE_EXPORT KisRecalculateGeneratorLayerJob : public KisSpontaneousJob
{
public:
    KisRecalculateGeneratorLayerJob(KisGeneratorLayerSP layer);

    bool overrides(const KisSpontaneousJob *otherJob) override;
    void run() override;
    int levelOfDetail() const override;

    QString debugName() const override;

private:
    KisGeneratorLayerSP m_layer;
};

#endif /* __KIS_RECALCULATE_GENERATOR_LAYER_JOB_H */
