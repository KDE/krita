/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRUNNABLESTROKEJOBDATABASE_H
#define KISRUNNABLESTROKEJOBDATABASE_H


#include "kritaimage_export.h"
#include "kis_stroke_job_strategy.h"
#include "kis_runnable.h"

class KRITAIMAGE_EXPORT KisRunnableStrokeJobDataBase : public KisStrokeJobData, public KisRunnable
{
public:
    KisRunnableStrokeJobDataBase(KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                                 KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);
};

#endif // KISRUNNABLESTROKEJOBDATABASE_H
