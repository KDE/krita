/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMOVEBOUNDSCALCULATIONJOB_H
#define KISMOVEBOUNDSCALCULATIONJOB_H

#include <QObject>
#include "kis_spontaneous_job.h"
#include "kis_types.h"
#include "kis_selection.h"

class KisMoveBoundsCalculationJob : public QObject, public KisSpontaneousJob
{
    Q_OBJECT
public:
    KisMoveBoundsCalculationJob(KisNodeList nodes, KisSelectionSP selection, QObject *requestedBy);

    void run() override;
    bool overrides(const KisSpontaneousJob *otherJob) override;
    int levelOfDetail() const override;

    QString debugName() const override;

Q_SIGNALS:
    void sigCalcualtionFinished(const QRect &bounds);

private:
    KisNodeList m_nodes;
    KisSelectionSP m_selection;
    QObject *m_requestedBy;
};

#endif // KISMOVEBOUNDSCALCULATIONJOB_H
