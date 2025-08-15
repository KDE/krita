/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_switch_current_time_command.h"

#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_command_ids.h"
#include "kis_keyframe_channel.h"
#include "kis_node.h"


KisSwitchCurrentTimeCommand::KisSwitchCurrentTimeCommand(KisImageAnimationInterface *animation, int oldTime, int newTime, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Switch current time"), parent),
      m_animation(animation),
      m_oldTime(oldTime),
      m_newTime(newTime)
{
}

KisSwitchCurrentTimeCommand::~KisSwitchCurrentTimeCommand()
{
}

int KisSwitchCurrentTimeCommand::id() const
{
    return KisCommandUtils::ChangeCurrentTimeId;
}

bool KisSwitchCurrentTimeCommand::mergeWith(const KUndo2Command* command)
{
    const KisSwitchCurrentTimeCommand *other =
        dynamic_cast<const KisSwitchCurrentTimeCommand*>(command);

    if (!other || other->id() != id()) {
        return false;
    }

    m_newTime = other->m_newTime;
    return true;
}

void KisSwitchCurrentTimeCommand::redo()
{
    m_animation->requestTimeSwitchNonGUI(m_newTime);
}

void KisSwitchCurrentTimeCommand::undo()
{
    m_animation->requestTimeSwitchNonGUI(m_oldTime);
}

// ====================================//

KisSwitchCurrentTimeToKeyframeCommand::KisSwitchCurrentTimeToKeyframeCommand(KisImageAnimationInterface *animation, int oldTime, KisNodeSP node, KoID channelId, KisKeyframeSP targetKeyframe, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Switch current time to keyframe"), parent),
      m_animation(animation),
      m_oldTime(oldTime),
      m_node(node),
      m_channelId(channelId),
      m_targetKeyframe(targetKeyframe)
{
}

KisSwitchCurrentTimeToKeyframeCommand::~KisSwitchCurrentTimeToKeyframeCommand()
{
}

int KisSwitchCurrentTimeToKeyframeCommand::id() const
{
    return KisCommandUtils::ChangeCurrentTimeToKeyId;
}

bool KisSwitchCurrentTimeToKeyframeCommand::mergeWith(const KUndo2Command* command)
{
    // Note: Maybe we could merge this with a regular time switch command with a bit more logic.
    const KisSwitchCurrentTimeToKeyframeCommand *other =
        dynamic_cast<const KisSwitchCurrentTimeToKeyframeCommand*>(command);

    if (!other || other->id() != id()) {
        return false;
    }

    m_targetKeyframe = other->m_targetKeyframe;
    return true;
}

void KisSwitchCurrentTimeToKeyframeCommand::redo()
{
    KisKeyframeChannel *channel = m_node->getKeyframeChannel(m_channelId.id());
    if (channel) {
        int newTime = channel->lookupKeyframeTime(m_targetKeyframe);
        m_animation->requestTimeSwitchNonGUI(newTime);
    }
}

void KisSwitchCurrentTimeToKeyframeCommand::undo()
{
    m_animation->requestTimeSwitchNonGUI(m_oldTime);
}
