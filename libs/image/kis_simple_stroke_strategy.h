/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIMPLE_STROKE_STRATEGY_H
#define __KIS_SIMPLE_STROKE_STRATEGY_H

#include <QVector>
#include "kis_stroke_strategy.h"
#include "kis_stroke_job_strategy.h"


class KRITAIMAGE_EXPORT KisSimpleStrokeStrategy : public KisStrokeStrategy
{
public:
    enum JobType {
        JOB_INIT = 0,
        JOB_CANCEL,
        JOB_FINISH,
        JOB_DOSTROKE,
        JOB_SUSPEND,
        JOB_RESUME,

        NJOBS
    };

public:
    KisSimpleStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name = KUndo2MagicString());

    KisStrokeJobStrategy* createInitStrategy() override;
    KisStrokeJobStrategy* createFinishStrategy() override;
    KisStrokeJobStrategy* createCancelStrategy() override;
    KisStrokeJobStrategy* createDabStrategy() override;
    KisStrokeJobStrategy* createSuspendStrategy() override;
    KisStrokeJobStrategy* createResumeStrategy() override;

    KisStrokeJobData* createInitData() override;
    KisStrokeJobData* createFinishData() override;
    KisStrokeJobData* createCancelData() override;
    KisStrokeJobData* createSuspendData() override;
    KisStrokeJobData* createResumeData() override;

    virtual void initStrokeCallback();
    virtual void finishStrokeCallback();
    virtual void cancelStrokeCallback();
    virtual void doStrokeCallback(KisStrokeJobData *data);
    virtual void suspendStrokeCallback();
    virtual void resumeStrokeCallback();

    static QLatin1String jobTypeToString(JobType type);

protected:
    void enableJob(JobType type, bool enable = true,
                   KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                   KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

protected:
    KisSimpleStrokeStrategy(const KisSimpleStrokeStrategy &rhs);

private:
    KisStrokeJobStrategy* createStrategy(JobType type);
    KisStrokeJobData* createData(JobType type);

private:
    QVector<bool> m_jobEnabled;
    QVector<KisStrokeJobData::Sequentiality> m_jobSequentiality;
    QVector<KisStrokeJobData::Exclusivity> m_jobExclusivity;
};

#endif /* __KIS_SIMPLE_STROKE_STRATEGY_H */
