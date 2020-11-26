/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
      m_forceLodModeIfPossible(false),
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
      m_forceLodModeIfPossible(rhs.m_forceLodModeIfPossible),
      m_balancingRatioOverride(rhs.m_balancingRatioOverride),
      m_id(rhs.m_id),
      m_name(rhs.m_name),
      m_mutatedJobsInterface(0)
{
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_strokeId && !m_mutatedJobsInterface &&
                            "After the stroke has been started, no copying must happen");
}

bool KisStrokeStrategy::forceLodModeIfPossible() const
{
    return m_forceLodModeIfPossible;
}

void KisStrokeStrategy::setForceLodModeIfPossible(bool value)
{
    m_forceLodModeIfPossible = value;
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

KisStrokeStrategy *KisStrokeStrategy::createLegacyInitializingStroke()
{
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

KisLodPreferences KisStrokeStrategy::currentLodPreferences() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_mutatedJobsInterface, KisLodPreferences());
    return m_mutatedJobsInterface->lodPreferences();
}

void KisStrokeStrategy::setMutatedJobsInterface(KisStrokesQueueMutatedJobInterface *mutatedJobsInterface, KisStrokeId strokeId)
{
    m_mutatedJobsInterface = mutatedJobsInterface;
    m_strokeId = strokeId;
}

void KisStrokeStrategy::addMutatedJobs(const QVector<KisStrokeJobData *> list)
{
    KIS_SAFE_ASSERT_RECOVER(m_mutatedJobsInterface && m_strokeId) {
        qDeleteAll(list);
        return;
    }

    m_mutatedJobsInterface->addMutatedJobs(m_strokeId, list);
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
