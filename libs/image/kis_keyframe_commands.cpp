
#include "kis_keyframe_commands.h"

#include <algorithm>
#include <kis_pointer_utils.h>
#include <KisCollectionUtils.h>
#include "kis_time_range.h"
#include "kis_animation_cycle.h"

using CycleSP = QSharedPointer<KisAnimationCycle>;

KisReplaceKeyframeCommand::KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeBaseSP keyframe,
                                                     KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_channel(channel)
    , m_keyframe(keyframe)
    , m_newTime(time)
{}

void KisReplaceKeyframeCommand::redo() {
    if (m_newTime >= 0) {
        m_overwrittenKeyframe = m_channel->itemAt(m_newTime);

        if (m_overwrittenKeyframe) {
            m_channel->removeKeyframeLogical(m_overwrittenKeyframe);
        }
    }

    if (m_keyframe) {
        const bool currentlyOnChannel = m_channel->itemAt(m_keyframe->time()) == m_keyframe;
        m_oldTime = currentlyOnChannel ? m_keyframe->time() : -1;
    }

    moveKeyframeTo(m_newTime);
}

void KisReplaceKeyframeCommand::undo() {
    moveKeyframeTo(m_oldTime);

    if (m_overwrittenKeyframe) {
        m_overwrittenKeyframe->setTime(m_newTime);
        m_channel->insertKeyframeLogical(m_overwrittenKeyframe);
        m_overwrittenKeyframe = nullptr;
    }
}

void KisReplaceKeyframeCommand::moveKeyframeTo(int dstTime)
{
    if (!m_keyframe) return;

    const bool currentlyOnChannel = m_channel->itemAt(m_keyframe->time()) == m_keyframe;

    if (dstTime < 0) {
        if (currentlyOnChannel) {
            m_channel->removeKeyframeLogical(m_keyframe);
        }
    } else {
        if (currentlyOnChannel) {
            m_channel->moveKeyframeImpl(m_keyframe, dstTime);
        } else {
            m_keyframe->setTime(dstTime);
            m_channel->insertKeyframeLogical(m_keyframe);
        }
    }
}

KisSwapFramesCommand::KisSwapFramesCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP lhsFrame, KisKeyframeBaseSP rhsFrame, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(channel),
      m_lhsFrame(lhsFrame),
      m_rhsFrame(rhsFrame)
{
}

void KisSwapFramesCommand::redo()
{
    m_channel->swapKeyframesImpl(m_lhsFrame, m_rhsFrame);
}

void KisSwapFramesCommand::undo()
{
    m_channel->swapKeyframesImpl(m_lhsFrame, m_rhsFrame);
}

KisDefineCycleCommand::KisDefineCycleCommand(CycleSP oldCycle, CycleSP newCycle, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_channel(oldCycle ? oldCycle->channel() : newCycle->channel())
    , m_oldCycle(oldCycle)
    , m_newCycle(newCycle)
{}

void KisDefineCycleCommand::redo()
{
    if (m_oldCycle) {
        m_channel->removeCycle(m_oldCycle);
    }

    if (m_newCycle) {
        m_channel->addCycle(m_newCycle);
    }
}

void KisDefineCycleCommand::undo()
{
    if (m_newCycle) {
        m_channel->removeCycle(m_newCycle);
    }

    if (m_oldCycle) {
        m_channel->addCycle(m_oldCycle);
    }
}

QSharedPointer<KisAnimationCycle> KisDefineCycleCommand::cycle() const
{
    return m_newCycle;
}
