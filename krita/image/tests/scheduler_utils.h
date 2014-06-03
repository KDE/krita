/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __SCHEDULER_UTILS_H
#define __SCHEDULER_UTILS_H

#include <QRect>
#include "kis_merge_walker.h"
#include "kis_stroke_strategy.h"
#include "kis_stroke_job.h"
#include "kis_spontaneous_job.h"
#include "kis_stroke.h"


#define SCOMPARE(s1, s2) QCOMPARE(QString(s1), QString(s2))

#define COMPARE_WALKER(item, walker)            \
    QCOMPARE(item->walker(), walker)
#define COMPARE_NAME(item, name)                                \
    QCOMPARE(getJobName(item->strokeJob()), QString(name))
#define VERIFY_EMPTY(item)                                      \
    QVERIFY(!item->isRunning())

void executeStrokeJobs(KisStroke *stroke) {
    KisStrokeJob *job;

    while((job = stroke->popOneJob())) {
        job->run();
        delete job;
    }
}

bool checkWalker(KisBaseRectsWalkerSP walker, const QRect &rect) {
    if(walker->requestedRect() == rect) {
        return true;
    }
    else {
        qDebug() << "walker rect:" << walker->requestedRect();
        qDebug() << "expected rect:" << rect;
        return false;
    }
}

class KisNoopSpontaneousJob : public KisSpontaneousJob
{
public:
    KisNoopSpontaneousJob(bool overridesEverything = false)
        : m_overridesEverything(overridesEverything)
    {
    }

    void run() {
    }

    bool overrides(const KisSpontaneousJob *otherJob) {
        Q_UNUSED(otherJob);
        return m_overridesEverything;
    }

private:
    bool m_overridesEverything;
};

class KisNoopDabStrategy : public KisStrokeJobStrategy
{
public:
KisNoopDabStrategy(QString name)
        : m_name(name)
    {}

    void run(KisStrokeJobData *data) {
        Q_UNUSED(data);
    }

    QString name() {
        return m_name;
    }

private:
    QString m_name;
};

class KisTestingStrokeStrategy : public KisStrokeStrategy
{
public:
    KisTestingStrokeStrategy(const QString &prefix = QString(),
                             bool exclusive = false,
                             bool inhibitServiceJobs = false)
        : m_prefix(prefix),
          m_inhibitServiceJobs(inhibitServiceJobs)
    {
        setExclusive(exclusive);
    }

    KisStrokeJobStrategy* createInitStrategy() {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "init") : 0;
    }

    KisStrokeJobStrategy* createFinishStrategy() {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "finish") : 0;
    }

    KisStrokeJobStrategy* createCancelStrategy() {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "cancel") : 0;
    }

    KisStrokeJobStrategy* createDabStrategy() {
        return new KisNoopDabStrategy(m_prefix + "dab");
    }

private:
    QString m_prefix;
    bool m_inhibitServiceJobs;
};

inline QString getJobName(KisStrokeJob *job) {
    KisNoopDabStrategy *pointer =
        dynamic_cast<KisNoopDabStrategy*>(job->testingGetDabStrategy());
    Q_ASSERT(pointer);

    return pointer->name();
}

#endif /* __SCHEDULER_UTILS_H */
