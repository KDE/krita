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

void KisIdleTaskStrokeStrategy::initStrokeCallback()
{
    m_idleStrokeTime.start();
}

void KisIdleTaskStrokeStrategy::finishStrokeCallback()
{
    Q_EMIT sigIdleTaskFinished();

    /**
     * Just a small sanity check if idle tasks don't occupy too much time
     * and don't interfere with user's workflow. Theoretically, even if the
     * time consumed is too high, it may still be acceptable if the task is
     * split in multiple jobs, which would be cancelled but the user's action.
     */
    const qint64 elapsedTime = m_idleStrokeTime.elapsed();

    if (elapsedTime > preferredIdleTaskMaximumTime()) {
        qWarning() << "WARNING: idle task consumed more than" <<
                      preferredIdleTaskMaximumTime() <<
                      "ms, it can cause visible distractions to the user";
        qWarning() << "WARNING: time consumed in" << id() <<
                      "is" << elapsedTime << "ms";
    }
}

QWeakPointer<boost::none_t> KisIdleTaskStrokeStrategy::idleTaskCookie()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_idleTaskCookie, m_idleTaskCookie);
    m_idleTaskCookie.reset(new boost::none_t(boost::none));
    return m_idleTaskCookie;
}

int KisIdleTaskStrokeStrategy::preferredIdleTaskMaximumTime()
{
    /**
     * Sometimes (unless split into multiple jobs) idle tasks can interfere
     * with user actions. We have a sanity check that checks if the tasks are
     * quick enough to not interfere into the user's actions. This time limit
     * is the maximum "allowed" idle task size.
     *
     * Definition of "allowed" here is not strict, since the tasks are not
     * cancelled when reaching the limit. Just a warning is spit into the
     * terminal.
     */

    return 200; // ms
}

int KisIdleTaskStrokeStrategy::preferredIdleWatcherInterval()
{
    /**
     * Preferred idle watcher interval for checks of the image idleness
     * state. Idle watcher checks the image four times before starting an
     * idle task, so the actual idle task delay is
     * `4 * preferredIdleWatcherInterval()` milliseconds.
     */
    return 50; // ms
}

