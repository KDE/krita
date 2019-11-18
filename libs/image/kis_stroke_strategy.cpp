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

#include "kis_stroke_strategy.h"
#include <KoCompositeOpRegistry.h>
#include "kis_stroke_job_strategy.h"
#include "KisStrokesQueueMutatedJobInterface.h"


KisStrokeStrategy::KisStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name)
    : m_exclusive(false),
      m_supportsWrapAroundMode(false),
      m_clearsRedoOnStart(true),
      m_requestsOtherStrokesToEnd(true),
      m_canForgetAboutMe(false),
      m_needsExplicitCancel(false),
      m_balancingRatioOverride(-1.0),
      m_id(id),
      m_name(name),
      m_mutatedJobsInterface(0)
{
}

KisStrokeStrategy::KisStrokeStrategy(const KisStrokeStrategy &rhs)
    : m_exclusive(rhs.m_exclusive),
      m_supportsWrapAroundMode(rhs.m_supportsWrapAroundMode),
      m_clearsRedoOnStart(rhs.m_clearsRedoOnStart),
      m_requestsOtherStrokesToEnd(rhs.m_requestsOtherStrokesToEnd),
      m_canForgetAboutMe(rhs.m_canForgetAboutMe),
      m_needsExplicitCancel(rhs.m_needsExplicitCancel),
      m_balancingRatioOverride(rhs.m_balancingRatioOverride),
      m_id(rhs.m_id),
      m_name(rhs.m_name),
      m_mutatedJobsInterface(0)
{
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_cancelStrokeId && !m_mutatedJobsInterface &&
                            "After the stroke has been started, no copying must happen");
}

KisStrokeStrategy::~KisStrokeStrategy()
{
}

void KisStrokeStrategy::notifyUserStartedStroke()
{
}

void KisStrokeStrategy::notifyUserEndedStroke()
{
}

KisStrokeJobStrategy* KisStrokeStrategy::createInitStrategy()
{
    return 0;
}

KisStrokeJobStrategy* KisStrokeStrategy::createFinishStrategy()
{
    return 0;
}

KisStrokeJobStrategy* KisStrokeStrategy::createCancelStrategy()
{
    return 0;
}

KisStrokeJobStrategy* KisStrokeStrategy::createDabStrategy()
{
    return 0;
}

KisStrokeJobStrategy* KisStrokeStrategy::createSuspendStrategy()
{
    return 0;
}

KisStrokeJobStrategy* KisStrokeStrategy::createResumeStrategy()
{
    return 0;
}

KisStrokeJobData* KisStrokeStrategy::createInitData()
{
    return 0;
}

KisStrokeJobData* KisStrokeStrategy::createFinishData()
{
    return 0;
}

KisStrokeJobData* KisStrokeStrategy::createCancelData()
{
    return 0;
}

KisStrokeJobData* KisStrokeStrategy::createSuspendData()
{
    return 0;
}

KisStrokeJobData* KisStrokeStrategy::createResumeData()
{
    return 0;
}

KisStrokeStrategy* KisStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);
    return 0;
}

bool KisStrokeStrategy::isExclusive() const
{
    return m_exclusive;
}

bool KisStrokeStrategy::supportsWrapAroundMode() const
{
    return m_supportsWrapAroundMode;
}

QString KisStrokeStrategy::id() const
{
    return m_id;
}

KUndo2MagicString KisStrokeStrategy::name() const
{
    return m_name;
}

void KisStrokeStrategy::setMutatedJobsInterface(KisStrokesQueueMutatedJobInterface *mutatedJobsInterface)
{
    m_mutatedJobsInterface = mutatedJobsInterface;
}

void KisStrokeStrategy::addMutatedJobs(const QVector<KisStrokeJobData *> list)
{
    KIS_SAFE_ASSERT_RECOVER(m_mutatedJobsInterface && m_cancelStrokeId) {
        qDeleteAll(list);
        return;
    }

    m_mutatedJobsInterface->addMutatedJobs(m_cancelStrokeId, list);
}

void KisStrokeStrategy::addMutatedJob(KisStrokeJobData *data)
{
    addMutatedJobs({data});
}

void KisStrokeStrategy::setExclusive(bool value)
{
    m_exclusive = value;
}

void KisStrokeStrategy::setSupportsWrapAroundMode(bool value)
{
    m_supportsWrapAroundMode = value;
}

bool KisStrokeStrategy::clearsRedoOnStart() const
{
    return m_clearsRedoOnStart;
}

void KisStrokeStrategy::setClearsRedoOnStart(bool value)
{
    m_clearsRedoOnStart = value;
}

bool KisStrokeStrategy::requestsOtherStrokesToEnd() const
{
    return m_requestsOtherStrokesToEnd;
}

void KisStrokeStrategy::setRequestsOtherStrokesToEnd(bool value)
{
    m_requestsOtherStrokesToEnd = value;
}

bool KisStrokeStrategy::canForgetAboutMe() const
{
    return m_canForgetAboutMe;
}

void KisStrokeStrategy::setCanForgetAboutMe(bool value)
{
    m_canForgetAboutMe = value;
}

bool KisStrokeStrategy::needsExplicitCancel() const
{
    return m_needsExplicitCancel;
}

void KisStrokeStrategy::setNeedsExplicitCancel(bool value)
{
    m_needsExplicitCancel = value;
}

qreal KisStrokeStrategy::balancingRatioOverride() const
{
    return m_balancingRatioOverride;
}

void KisStrokeStrategy::setBalancingRatioOverride(qreal value)
{
    m_balancingRatioOverride = value;
}
