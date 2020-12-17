/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_UPDATE_SELECTION_JOB_H
#define __KIS_UPDATE_SELECTION_JOB_H

#include "kis_spontaneous_job.h"
#include "kis_selection.h"

class KRITAIMAGE_EXPORT KisUpdateSelectionJob : public KisSpontaneousJob
{
public:
    KisUpdateSelectionJob(KisSelectionSP selection, const QRect &updateRect = QRect());

    bool overrides(const KisSpontaneousJob *otherJob) override;
    void run() override;
    int levelOfDetail() const override;
    QString debugName() const override;

private:
    KisSelectionSP m_selection;
    QRect m_updateRect;
};

#endif /* __KIS_UPDATE_SELECTION_JOB_H */
