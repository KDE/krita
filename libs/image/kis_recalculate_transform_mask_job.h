/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_RECALCULATE_TRANSFORM_MASK_JOB_H
#define __KIS_RECALCULATE_TRANSFORM_MASK_JOB_H

#include "kis_types.h"
#include "kis_spontaneous_job.h"


class KRITAIMAGE_EXPORT KisRecalculateTransformMaskJob : public KisSpontaneousJob
{
public:
    KisRecalculateTransformMaskJob(KisTransformMaskSP mask);

    bool overrides(const KisSpontaneousJob *otherJob) override;
    void run() override;
    int levelOfDetail() const override;

    QString debugName() const override;

private:
    KisTransformMaskSP m_mask;
};

#endif /* __KIS_RECALCULATE_TRANSFORM_MASK_JOB_H */
