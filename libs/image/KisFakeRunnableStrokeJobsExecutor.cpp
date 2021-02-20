/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFakeRunnableStrokeJobsExecutor.h"

#include <KisRunnableStrokeJobData.h>
#include <kis_assert.h>

#include <QVector>

void KisFakeRunnableStrokeJobsExecutor::addRunnableJobs(const QVector<KisRunnableStrokeJobDataBase *> &list)
{
    Q_FOREACH (KisRunnableStrokeJobDataBase *data, list) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(data->sequentiality() != KisStrokeJobData::BARRIER && "barrier jobs are not supported on the fake executor");
        KIS_SAFE_ASSERT_RECOVER_NOOP(data->exclusivity() != KisStrokeJobData::EXCLUSIVE && "exclusive jobs are not supported on the fake executor");

        data->run();
    }

    qDeleteAll(list);
}
