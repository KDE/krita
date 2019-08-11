/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
