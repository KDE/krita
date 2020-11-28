/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_switch_time_stroke_strategy.h"

#include <QMutex>

#include "kis_image_animation_interface.h"
#include "kis_post_execution_undo_adapter.h"
#include "commands_new/kis_switch_current_time_command.h"


struct KisSwitchTimeStrokeStrategy::Private
{
    Private(int frameId, bool needsRegeneration)
        : token(new SharedToken(frameId, needsRegeneration))
    {
    }

    KisImageAnimationInterface *interface;
    KisPostExecutionUndoAdapter *undoAdapter;
    SharedTokenSP token;
};

KisSwitchTimeStrokeStrategy::KisSwitchTimeStrokeStrategy(int frameId,
                                                         bool needsRegeneration,
                                                         KisImageAnimationInterface *interface,
                                                         KisPostExecutionUndoAdapter *undoAdapter)
    : KisSimpleStrokeStrategy(QLatin1String("switch_current_frame_stroke"), kundo2_i18n("Switch Frames")),
      m_d(new Private(frameId, needsRegeneration))
{
    m_d->interface = interface;
    m_d->undoAdapter = undoAdapter;


    enableJob(JOB_INIT, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    // switching frames is a distinct user action, so it should
    // cancel the playback or any action easily
    setRequestsOtherStrokesToEnd(true);
    setClearsRedoOnStart(false);
}

KisSwitchTimeStrokeStrategy::~KisSwitchTimeStrokeStrategy()
{
}
void KisSwitchTimeStrokeStrategy::initStrokeCallback()
{
    const int frameId = m_d->token->fetchTime();

    if (frameId == m_d->interface->currentTime()) return;

    const int oldTime = m_d->interface->currentTime();
    m_d->interface->explicitlySetCurrentTime(frameId);

    if (m_d->undoAdapter) {
        KUndo2CommandSP cmd(
            new KisSwitchCurrentTimeCommand(m_d->interface,
                                            oldTime,
                                            frameId));
        m_d->undoAdapter->addCommand(cmd);
    }
}
KisStrokeStrategy* KisSwitchTimeStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);

    // This stroke is explicitly made LEGACY, so that it could create a barrier for
    // time switch. Theoretically, we can have separate time ids for Lod0 and LodN,
    // but currently this idea doesn't wotk for some reason. The consequences of
    // making this stroke a barrier:
    //
    // * LodSync stroke is started after every time switch (not slow, but still...)
    // * The frame switches only *after* all the pending strokes are finished
    // * LodSync stroke is started even when the image is not changed (not regeneration needed)!

    return 0;
}

KisSwitchTimeStrokeStrategy::SharedTokenSP KisSwitchTimeStrokeStrategy::token() const
{
    return m_d->token;
}


/*****************************************************************/
/*        KisSwitchTimeStrokeStrategy::SharedToken               */
/*****************************************************************/

struct KisSwitchTimeStrokeStrategy::SharedToken::Private {
    Private(int _time, bool _needsRegeneration)
        : time(_time),
          needsRegeneration(_needsRegeneration),
          isCompleted(false)
    {
    }

    QMutex mutex;
    int time;
    bool needsRegeneration;
    bool isCompleted;
};

KisSwitchTimeStrokeStrategy::SharedToken::SharedToken(int initialTime, bool needsRegeneration)
    : m_d(new Private(initialTime, needsRegeneration))
{
}

KisSwitchTimeStrokeStrategy::SharedToken::~SharedToken()
{
}

bool KisSwitchTimeStrokeStrategy::SharedToken::tryResetDestinationTime(int time, bool needsRegeneration)
{
    QMutexLocker l(&m_d->mutex);

    const bool result =
        !m_d->isCompleted &&
        (m_d->needsRegeneration || !needsRegeneration);

    if (result) {
        m_d->time = time;
    }

    return result;
}

int KisSwitchTimeStrokeStrategy::SharedToken::fetchTime() const
{
    QMutexLocker l(&m_d->mutex);
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->isCompleted);

    m_d->isCompleted = true;
    return m_d->time;
}
