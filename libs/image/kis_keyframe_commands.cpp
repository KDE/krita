
#include "kis_keyframe_commands.h"

KisReplaceKeyframeCommand::KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeSP keyframe, KUndo2Command *parentCommand)
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


KisMoveFrameCommand::KisMoveFrameCommand(KisKeyframeChannel *channel, KisKeyframeSP keyframe, int oldTime, int newTime, KUndo2Command *parentCommand)
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
