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
#include "kis_image.h"


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

bool checkWalker(KisBaseRectsWalkerSP walker, const QRect &rect, int lod = 0) {
    if(walker->requestedRect() == rect && walker->levelOfDetail() == lod) {
        return true;
    }
    else {
        dbgKrita << "walker rect:" << walker->requestedRect();
        dbgKrita << "expected rect:" << rect;
        dbgKrita << "walker lod:" << walker->levelOfDetail();
        dbgKrita << "expected lod:" << lod;
        return false;
    }
}

class KisNoopSpontaneousJob : public KisSpontaneousJob
{
public:
    KisNoopSpontaneousJob(bool overridesEverything = false, int lod = 0)
        : m_overridesEverything(overridesEverything),
          m_lod(lod)
    {
    }

    void run() override {
    }

    bool overrides(const KisSpontaneousJob *otherJob) override {
        Q_UNUSED(otherJob);
        return m_overridesEverything;
    }

    int levelOfDetail() const override {
        return m_lod;
    }

    QString debugName() const override {
        return "KisNoopSpontaneousJob";
    }

private:
    bool m_overridesEverything;
    int m_lod;
};

static QStringList globalExecutedDabs;

class KisNoopDabStrategy : public KisStrokeJobStrategy
{
public:
    KisNoopDabStrategy(QString name)
    : m_name(name),
      m_isMarked(false)
    {}

    void run(KisStrokeJobData *data) override {
        Q_UNUSED(data);

        globalExecutedDabs << m_name;
    }

    virtual QString name(KisStrokeJobData *data) const {
        Q_UNUSED(data);
        return m_name;
    }

    void setMarked() {
        m_isMarked = true;
    }

    bool isMarked() const {
        return m_isMarked;
    }

    QString debugId() const override {
        return "KisNoopDabStrategy";
    }

private:
    QString m_name;
    bool m_isMarked;
};

class KisTestingStrokeJobData : public KisStrokeJobData
{
public:
    KisTestingStrokeJobData(Sequentiality sequentiality = SEQUENTIAL,
                            Exclusivity exclusivity = NORMAL,
                            bool addMutatedJobs = false,
                            const QString &customSuffix = QString())
        : KisStrokeJobData(sequentiality, exclusivity),
          m_addMutatedJobs(addMutatedJobs),
          m_customSuffix(customSuffix)
    {
    }

    KisTestingStrokeJobData(const KisTestingStrokeJobData &rhs)
        : KisStrokeJobData(rhs),
          m_addMutatedJobs(rhs.m_addMutatedJobs)
    {
    }

    KisStrokeJobData* createLodClone(int levelOfDetail) override {
        Q_UNUSED(levelOfDetail);
        return new KisTestingStrokeJobData(*this);
    }

    bool m_addMutatedJobs = false;
    bool m_isMutated = false;
    QString m_customSuffix;
};

class KisMutatableDabStrategy : public KisNoopDabStrategy
{
public:
    KisMutatableDabStrategy(const QString &name, KisStrokeStrategy *parentStrokeStrategy)
        : KisNoopDabStrategy(name),
          m_parentStrokeStrategy(parentStrokeStrategy)
    {
    }

    void run(KisStrokeJobData *data) override {
        KisTestingStrokeJobData *td = dynamic_cast<KisTestingStrokeJobData*>(data);

        if (td && td->m_isMutated) {
            globalExecutedDabs << QString("%1_mutated").arg(name(data));
        } else if (td && td->m_addMutatedJobs) {
            globalExecutedDabs << name(data);

            for (int i = 0; i < 3; i++) {
                KisTestingStrokeJobData *newData =
                    new KisTestingStrokeJobData(td->sequentiality(), td->exclusivity(), false);
                newData->m_isMutated = true;
                m_parentStrokeStrategy->addMutatedJob(newData);
            }
        } else {
            globalExecutedDabs << name(data);
        }
    }

    virtual QString name(KisStrokeJobData *data) const override {
        const QString baseName = KisNoopDabStrategy::name(data);

        KisTestingStrokeJobData *td = dynamic_cast<KisTestingStrokeJobData*>(data);
        return !td || td->m_customSuffix.isEmpty() ? baseName : QString("%1_%2").arg(baseName).arg(td->m_customSuffix);
    }

    QString debugId() const override {
        return "KisMutatableDabStrategy";
    }

private:
    KisStrokeStrategy *m_parentStrokeStrategy = 0;
};


class KisTestingStrokeStrategy : public KisStrokeStrategy
{
public:
    KisTestingStrokeStrategy(const QLatin1String &prefix = QLatin1String(),
                             bool exclusive = false,
                             bool inhibitServiceJobs = false,
                             bool forceAllowInitJob = false,
                             bool forceAllowCancelJob = false)
        : KisStrokeStrategy(prefix, kundo2_noi18n(prefix)),
          m_prefix(prefix),
          m_inhibitServiceJobs(inhibitServiceJobs),
          m_forceAllowInitJob(forceAllowInitJob),
          m_forceAllowCancelJob(forceAllowCancelJob),
          m_cancelSeqNo(0)
    {
        setExclusive(exclusive);
    }

    KisTestingStrokeStrategy(const KisTestingStrokeStrategy &rhs, int levelOfDetail)
        : KisStrokeStrategy(rhs),
          m_prefix(rhs.m_prefix),
          m_inhibitServiceJobs(rhs.m_inhibitServiceJobs),
          m_forceAllowInitJob(rhs.m_forceAllowInitJob),
          m_cancelSeqNo(rhs.m_cancelSeqNo)
    {
        m_prefix = QString("clone%1_%2").arg(levelOfDetail).arg(m_prefix);
    }

    KisStrokeJobStrategy* createInitStrategy() override {
        return m_forceAllowInitJob || !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "init") : 0;
    }

    KisStrokeJobStrategy* createFinishStrategy() override {
        return !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "finish") : 0;
    }

    KisStrokeJobStrategy* createCancelStrategy() override {
        return m_forceAllowCancelJob || !m_inhibitServiceJobs ?
            new KisNoopDabStrategy(m_prefix + "cancel") : 0;
    }

    KisStrokeJobStrategy* createDabStrategy() override {
        return new KisMutatableDabStrategy(m_prefix + "dab", this);
    }

    KisStrokeStrategy* createLodClone(int levelOfDetail) override {
        return new KisTestingStrokeStrategy(*this, levelOfDetail);
    }

    class CancelData : public KisStrokeJobData
    {
    public:
        CancelData(int seqNo) : m_seqNo(seqNo) {}
        int seqNo() const { return m_seqNo; }

    private:
        int m_seqNo;
    };

    KisStrokeJobData* createCancelData() override {
        return new CancelData(m_cancelSeqNo++);
    }

private:
    QString m_prefix;
    bool m_inhibitServiceJobs;
    int m_forceAllowInitJob;
    bool m_forceAllowCancelJob;
    int m_cancelSeqNo;
};

inline QString getJobName(KisStrokeJob *job) {
    KisNoopDabStrategy *pointer =
        dynamic_cast<KisNoopDabStrategy*>(job->testingGetDabStrategy());
    KIS_ASSERT(pointer);

    return pointer->name(job->testingGetDabData());
}

inline int cancelSeqNo(KisStrokeJob *job) {
    KisTestingStrokeStrategy::CancelData *pointer =
        dynamic_cast<KisTestingStrokeStrategy::CancelData*>
        (job->testingGetDabData());
    Q_ASSERT(pointer);

    return pointer->seqNo();
}

#endif /* __SCHEDULER_UTILS_H */
