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

#include "KisRunnableBasedStrokeStrategy.h"

#include <QRunnable>
#include <functional>

#include "KisRunnableStrokeJobData.h"
#include "KisRunnableStrokeJobsInterface.h"

struct KisRunnableBasedStrokeStrategy::JobsInterface : public KisRunnableStrokeJobsInterface
{
    JobsInterface(KisRunnableBasedStrokeStrategy *q)
        : m_q(q)
    {
    }


    void addRunnableJobs(const QVector<KisRunnableStrokeJobDataBase*> &list) {
        QVector<KisStrokeJobData*> newList;

        Q_FOREACH (KisRunnableStrokeJobDataBase *item, list) {
            newList.append(item);
        }

        m_q->addMutatedJobs(newList);
    }

private:
    KisRunnableBasedStrokeStrategy *m_q;
};


KisRunnableBasedStrokeStrategy::KisRunnableBasedStrokeStrategy(QString id, const KUndo2MagicString &name)
    : KisSimpleStrokeStrategy(id, name),
      m_jobsInterface(new JobsInterface(this))
{
}

KisRunnableBasedStrokeStrategy::KisRunnableBasedStrokeStrategy(const KisRunnableBasedStrokeStrategy &rhs)
    : KisSimpleStrokeStrategy(rhs),
      m_jobsInterface(new JobsInterface(this))
{
}

KisRunnableBasedStrokeStrategy::~KisRunnableBasedStrokeStrategy()
{
}

void KisRunnableBasedStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (!data) return;

    KisRunnableStrokeJobDataBase *runnable = dynamic_cast<KisRunnableStrokeJobDataBase*>(data);
    if (!runnable) return;

    runnable->run();
}

KisRunnableStrokeJobsInterface *KisRunnableBasedStrokeStrategy::runnableJobsInterface() const
{
    return m_jobsInterface.data();
}
