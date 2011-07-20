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

#include "kis_simple_stroke_strategy.h"


/***************************************************************/
/*         private class: SimpleStrokeJobStrategy              */
/***************************************************************/

class SimpleStrokeJobStrategy : public KisStrokeJobStrategy
{
public:
    SimpleStrokeJobStrategy(KisSimpleStrokeStrategy::JobType type,
                            Sequentiality sequentiality,
                            Exclusivity exclusivity,
                            KisSimpleStrokeStrategy *parentStroke)
        : KisStrokeJobStrategy(sequentiality, exclusivity),
          m_type(type),
          m_parentStroke(parentStroke)
    {
    }

    void run(StrokeJobData *data) {
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
            m_parentStroke->doStrokeCallback(
                static_cast<KisSimpleStrokeStrategy::Data*>(data));
            break;
        }
    }

private:
    KisSimpleStrokeStrategy::JobType m_type;
    KisSimpleStrokeStrategy *m_parentStroke;
};


/***************************************************************/
/*                 KisSimpleStrokeStrategy                     */
/***************************************************************/

KisSimpleStrokeStrategy::KisSimpleStrokeStrategy(QString id, QString name)
    : KisStrokeStrategy(id, name),
      m_jobEnabled(4, false),
      m_sequentiality(KisStrokeJobStrategy::SEQUENTIAL)
{
}

void KisSimpleStrokeStrategy::enableJob(JobType type, bool enable)
{
    m_jobEnabled[(int)type] = enable;
}

void KisSimpleStrokeStrategy::setSequentiality(KisStrokeJobStrategy::Sequentiality sequentiality)
{
    m_sequentiality = sequentiality;
}

KisStrokeJobStrategy*
KisSimpleStrokeStrategy::createStrategy(JobType type,
                                        KisStrokeJobStrategy::Sequentiality sequentiality,
                                        KisStrokeJobStrategy::Exclusivity exclusivity)
{
    KisStrokeJobStrategy *strategy = 0;

    if(m_jobEnabled[(int)type]) {
        strategy = new SimpleStrokeJobStrategy(type, sequentiality,
                                               exclusivity, this);
    }

    return strategy;
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createInitStrategy()
{
    return createStrategy(JOB_INIT,
                          KisStrokeJobStrategy::SEQUENTIAL,
                          KisStrokeJobStrategy::NORMAL);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createFinishStrategy()
{
    return createStrategy(JOB_FINISH,
                          KisStrokeJobStrategy::SEQUENTIAL,
                          KisStrokeJobStrategy::NORMAL);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createCancelStrategy()
{
    return createStrategy(JOB_CANCEL,
                          KisStrokeJobStrategy::SEQUENTIAL,
                          KisStrokeJobStrategy::NORMAL);
}

KisStrokeJobStrategy* KisSimpleStrokeStrategy::createDabStrategy()
{
    return createStrategy(JOB_DOSTROKE,
                          m_sequentiality,
                          KisStrokeJobStrategy::NORMAL);
}


KisStrokeJobStrategy::StrokeJobData* KisSimpleStrokeStrategy::createInitData()
{
    return 0;
}

KisStrokeJobStrategy::StrokeJobData* KisSimpleStrokeStrategy::createFinishData()
{
    return 0;
}

KisStrokeJobStrategy::StrokeJobData* KisSimpleStrokeStrategy::createCancelData()
{
    return 0;
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

void KisSimpleStrokeStrategy::doStrokeCallback(Data *data)
{
    Q_UNUSED(data);
}
