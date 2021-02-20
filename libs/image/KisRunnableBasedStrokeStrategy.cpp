/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


KisRunnableBasedStrokeStrategy::KisRunnableBasedStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name)
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
