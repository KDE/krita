/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISFAKERUNNABLESTROKEJOBSEXECUTOR_H
#define KISFAKERUNNABLESTROKEJOBSEXECUTOR_H

#include "KisRunnableStrokeJobsInterface.h"


class KRITAIMAGE_EXPORT KisFakeRunnableStrokeJobsExecutor : public KisRunnableStrokeJobsInterface
{
public:
    void addRunnableJobs(const QVector<KisRunnableStrokeJobDataBase*> &list);
};

#endif // KISFAKERUNNABLESTROKEJOBSEXECUTOR_H
