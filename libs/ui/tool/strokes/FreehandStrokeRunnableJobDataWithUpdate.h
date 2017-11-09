/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
