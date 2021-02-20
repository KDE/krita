/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FREEHANDSTROKERUNNABLEJOBDATAWITHUPDATE_H
#define FREEHANDSTROKERUNNABLEJOBDATAWITHUPDATE_H

#include "KisRunnableStrokeJobData.h"


class FreehandStrokeRunnableJobDataWithUpdate : public KisRunnableStrokeJobData
{
public:
    FreehandStrokeRunnableJobDataWithUpdate(QRunnable *runnable, KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                                            KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL)
        : KisRunnableStrokeJobData(runnable, sequentiality, exclusivity)
    {
    }

    FreehandStrokeRunnableJobDataWithUpdate(std::function<void()> func, KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                                            KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL)
        : KisRunnableStrokeJobData(func, sequentiality, exclusivity)
    {
    }
};

#endif // FREEHANDSTROKERUNNABLEJOBDATAWITHUPDATE_H
