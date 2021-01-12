/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_JOB_H
#define __KIS_STROKE_JOB_H

#include "kis_runnable_with_debug_name.h"
#include "kis_stroke_job_strategy.h"

class KisStrokeJob : public KisRunnableWithDebugName
{
public:
    KisStrokeJob(KisStrokeJobStrategy *strategy,
                 KisStrokeJobData *data,
                 int levelOfDetail,
                 bool isOwnJob)
        : m_dabStrategy(strategy),
          m_dabData(data),
          m_levelOfDetail(levelOfDetail),
          m_isOwnJob(isOwnJob)
    {
    }

    ~KisStrokeJob() override {
        delete m_dabData;
    }

    void run() override {
        m_dabStrategy->run(m_dabData);
    }

    KisStrokeJobData::Sequentiality sequentiality() const {
        return m_dabData ? m_dabData->sequentiality() : KisStrokeJobData::SEQUENTIAL;
    }

    bool isSequential() const {
        // Default value is 'SEQUENTIAL'
        return m_dabData ? m_dabData->isSequential() : true;
    }

    bool isBarrier() const {
        // Default value is simply 'SEQUENTIAL', *not* 'BARRIER'
        return m_dabData ? m_dabData->isBarrier() : false;
    }

    bool isExclusive() const {
        // Default value is 'NORMAL'
        return m_dabData ? m_dabData->isExclusive() : false;
    }

    int levelOfDetail() const {
        return m_dabData && m_dabData->levelOfDetailOverride() >= 0 ?
            m_dabData->levelOfDetailOverride() : m_levelOfDetail;
    }

    bool isCancellable() const {
        return m_isOwnJob &&
            (!m_dabData || m_dabData->isCancellable());
    }

    bool isOwnJob() const {
        return m_isOwnJob;
    }

    QString debugName() const override {
        return m_dabStrategy->debugId();
    }

private:
    // for testing use only, do not use in real code
    friend QString getJobName(KisStrokeJob *job);
    friend QString getCommandName(KisStrokeJob *job);
    friend int cancelSeqNo(KisStrokeJob *job);

    KisStrokeJobStrategy* testingGetDabStrategy() {
        return m_dabStrategy;
    }

    KisStrokeJobData* testingGetDabData() {
        return m_dabData;
    }

private:
    // Shared between different jobs
    KisStrokeJobStrategy *m_dabStrategy;

    // Owned by the job
    KisStrokeJobData *m_dabData;

    int m_levelOfDetail;
    bool m_isOwnJob;
};

#endif /* __KIS_STROKE_JOB_H */
