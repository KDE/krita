/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_simple_stroke_strategy.h"


/***************************************************************/
/*         private class: SimpleStrokeJobStrategy              */
/***************************************************************/

class SimpleStrokeJobStrategy : public KisStrokeJobStrategy
{
public:
    SimpleStrokeJobStrategy(KisSimpleStrokeStrategy::JobType type,
                            KisSimpleStrokeStrategy *parentStroke)
        : m_type(type),
          m_parentStroke(parentStroke)
    {
    }

    void run(KisStrokeJobData *data) override {
        switch(m_type) {
        case KisSimpleStrokeStrategy::JOB_INIT:
            Q_UNUSED(data);
            m_parentStroke->initStrokeCallback();
            break;
        case KisSimpleStrokeStrategy::JOB_FINISH:
            Q_UNUSED(data);
            m_parentStroke->finishStrokeCallback();
            break;
        case KisSimpleStrokeStrategy::JOB_CANCEL:
            Q_UNUSED(data);
            m_parentStroke->cancelStrokeCallback();
            break;
        case KisSimpleStrokeStrategy::JOB_DOSTROKE:
            m_parentStroke->doStrokeCallback(data);
            break;
        case KisSimpleStrokeStrategy::JOB_SUSPEND:
            m_parentStroke->suspendStrokeCallback();
            break;
        case KisSimpleStrokeStrategy::JOB_RESUME:
            m_parentStroke->resumeStrokeCallback();
            break;
        default:
            break;
        }
    }

    QString debugId() const override {
        return QString("%1/%2")
            .arg(m_parentStroke->id())
            .arg(KisSimpleStrokeStrategy::jobTypeToString(m_type));
    }

private:
    KisSimpleStrokeStrategy::JobType m_type;
    KisSimpleStrokeStrategy *m_parentStroke;
};


/***************************************************************/
/*                 KisSimpleStrokeStrategy                     */
/***************************************************************/

KisSimpleStrokeStrategy::KisSimpleStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name)
    : KisStrokeStrategy(id, name),
      m_jobEnabled(NJOBS, false),
      m_jobSequentiality(NJOBS, KisStrokeJobData::SEQUENTIAL),
      m_jobExclusivity(NJOBS, KisStrokeJobData::NORMAL)
{
}

KisSimpleStrokeStrategy::KisSimpleStrokeStrategy(const KisSimpleStrokeStrategy &rhs)
    : KisStrokeStrategy(rhs),
      m_jobEnabled(rhs.m_jobEnabled),
      m_jobSequentiality(rhs.m_jobSequentiality),
      m_jobExclusivity(rhs.m_jobExclusivity)
{
}

void KisSimpleStrokeStrategy::enableJob(JobType type, bool enable,
                                        KisStrokeJobData::Sequentiality sequentiality,
                                        KisStrokeJobData::Exclusivity exclusivity)
{
    m_jobEnabled[(int)type] = enable;
    m_jobSequentiality[(int)type] = sequentiality;
    m_jobExclusivity[(int)type] = exclusivity;
}

KisStrokeJobStrategy*
KisSimpleStrokeStrategy::createStrategy(JobType type)
{
    KisStrokeJobStrategy *strategy = 0;

    if(m_jobEnabled[(int)type]) {
        strategy = new SimpleStrokeJobStrategy(type, this);
    }

    return strategy;
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createInitStrategy()
{
    return createStrategy(JOB_INIT);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createFinishStrategy()
{
    return createStrategy(JOB_FINISH);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createCancelStrategy()
{
    return createStrategy(JOB_CANCEL);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createDabStrategy()
{
    return createStrategy(JOB_DOSTROKE);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createSuspendStrategy()
{
    return createStrategy(JOB_SUSPEND);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createResumeStrategy()
{
    return createStrategy(JOB_RESUME);
}

KisStrokeJobData* KisSimpleStrokeStrategy::createData(JobType type)
{
    KisStrokeJobData::Sequentiality sequentiality = m_jobSequentiality[(int)type];
    KisStrokeJobData::Exclusivity exclusivity = m_jobExclusivity[(int)type];

    return new KisStrokeJobData(sequentiality, exclusivity);
}

KisStrokeJobData* KisSimpleStrokeStrategy::createInitData()
{
    return createData(JOB_INIT);
}

KisStrokeJobData* KisSimpleStrokeStrategy::createFinishData()
{
    return createData(JOB_FINISH);
}

KisStrokeJobData* KisSimpleStrokeStrategy::createCancelData()
{
    return createData(JOB_CANCEL);
}

KisStrokeJobData* KisSimpleStrokeStrategy::createSuspendData()
{
    return createData(JOB_SUSPEND);
}

KisStrokeJobData* KisSimpleStrokeStrategy::createResumeData()
{
    return createData(JOB_RESUME);
}

void KisSimpleStrokeStrategy::initStrokeCallback()
{
}

void KisSimpleStrokeStrategy::finishStrokeCallback()
{
}

void KisSimpleStrokeStrategy::cancelStrokeCallback()
{
}

void KisSimpleStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Q_UNUSED(data);
}

void KisSimpleStrokeStrategy::suspendStrokeCallback()
{
}

void KisSimpleStrokeStrategy::resumeStrokeCallback()
{
}

QLatin1String KisSimpleStrokeStrategy::jobTypeToString(KisSimpleStrokeStrategy::JobType type)
{
    QLatin1String result;

    switch (type) {
    case JOB_INIT:
        result = QLatin1String("init");
        break;
    case JOB_DOSTROKE:
        result = QLatin1String("dostroke");
        break;
    case JOB_FINISH:
        result = QLatin1String("finish");
        break;
    case JOB_CANCEL:
        result = QLatin1String("cancel");
        break;
    case JOB_SUSPEND:
        result = QLatin1String("suspend");
        break;
    case JOB_RESUME:
        result = QLatin1String("resume");
        break;
    case NJOBS:
        qFatal("Undefined stroke job type: %d", type);
    }

    return result;
}
