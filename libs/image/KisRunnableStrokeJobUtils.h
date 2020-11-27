/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRUNNABLESTROKEJOBUTILS_H
#define KISRUNNABLESTROKEJOBUTILS_H

#include <QVector>

#include "kis_stroke_job_strategy.h"
#include "KisRunnableStrokeJobData.h"

namespace KritaUtils
{

template <typename Func, typename Job>
void addJobSequential(QVector<Job*> &jobs, Func func) {
    jobs.append(new KisRunnableStrokeJobData(func, KisStrokeJobData::SEQUENTIAL));
}

template <typename Func, typename Job>
void addJobConcurrent(QVector<Job*> &jobs, Func func) {
    jobs.append(new KisRunnableStrokeJobData(func, KisStrokeJobData::CONCURRENT));
}

template <typename Func, typename Job>
void addJobBarrier(QVector<Job*> &jobs, Func func) {
    jobs.append(new KisRunnableStrokeJobData(func, KisStrokeJobData::BARRIER));
}

template <typename Func, typename Job>
void addJobUniquelyCuncurrent(QVector<Job*> &jobs, Func func) {
    jobs.append(new KisRunnableStrokeJobData(func, KisStrokeJobData::UNIQUELY_CONCURRENT));
}

}

#endif // KISRUNNABLESTROKEJOBUTILS_H
