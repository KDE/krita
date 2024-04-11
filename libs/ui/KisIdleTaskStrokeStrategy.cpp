/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisIdleTaskStrokeStrategy.h"


KisIdleTaskStrokeStrategy::KisIdleTaskStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name)
    : KisRunnableBasedStrokeStrategy(id, name)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(true);
}

KisIdleTaskStrokeStrategy::~KisIdleTaskStrokeStrategy() = default;


KisStrokeStrategy *KisIdleTaskStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);
    /**
     * We do not generate preview for Instant Preview mode. Even though we
     * could do that, it is not very needed, because KisIdleWatcher ensures
     * that overview preview is generated only when all the background jobs
     * are completed.
     *
     * The only thing we should do about Instant Preview is to avoid resetting
     * LoDN planes, when the thumbnail is running. Therefore we should return
     * a fake noop strategy as our LoDN clone (that is a marker of non-legacy
     * stroke for the scheduler)
     */
    return new KisSimpleStrokeStrategy(QLatin1String("KisIdleTaskStrokeStrategy_FakeLodN"));
}

void KisIdleTaskStrokeStrategy::finishStrokeCallback()
{
    Q_EMIT sigIdleTaskFinished();
}

QWeakPointer<boost::none_t> KisIdleTaskStrokeStrategy::idleTaskCookie()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_idleTaskCookie, m_idleTaskCookie);
    m_idleTaskCookie.reset(new boost::none_t(boost::none));
    return m_idleTaskCookie;
}
