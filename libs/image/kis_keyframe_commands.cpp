
#include "kis_keyframe_commands.h"

#include <kis_pointer_utils.h>
#include "kis_animation_cycle.h"

KisReplaceKeyframeCommand::KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(channel),
      m_time(time),
      m_keyframe(keyframe),
      m_existingKeyframe(0)
{
}

void KisReplaceKeyframeCommand::redo() {
    m_existingKeyframe = m_channel->replaceKeyframeAt(m_time, m_keyframe);
}

void KisReplaceKeyframeCommand::undo() {
    m_channel->replaceKeyframeAt(m_time, m_existingKeyframe);
}

KisMoveFrameCommand::KisMoveFrameCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP keyframe, int oldTime, int newTime, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(channel),
      m_keyframe(keyframe),
      m_oldTime(oldTime),
      m_newTime(newTime)
{
}

void KisMoveFrameCommand::redo() {
    m_channel->moveKeyframeImpl(m_keyframe, m_newTime);
}

void KisMoveFrameCommand::undo() {
    m_channel->moveKeyframeImpl(m_keyframe, m_oldTime);
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

KisDefineCycleCommand::KisDefineCycleCommand(KisKeyframeChannel *channel, QSharedPointer<KisAnimationCycle> cycle, bool undefine, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_channel(channel)
    , m_cycle(cycle)
    , m_undefine(undefine)
{}

void KisDefineCycleCommand::redo()
{
    if (m_undefine) {
        m_channel->removeCycle(m_cycle);
    } else {
        m_channel->addCycle(m_cycle);
    }
}

void KisDefineCycleCommand::undo()
{
    if (m_undefine) {
        m_channel->addCycle(m_cycle);
    } else {
        m_channel->removeCycle(m_cycle);
    }
}

QSharedPointer<KisAnimationCycle> KisDefineCycleCommand::cycle() const
{
    return m_cycle;
}
