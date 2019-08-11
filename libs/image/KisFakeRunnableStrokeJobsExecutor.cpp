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
