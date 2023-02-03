/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <klocalizedstring.h>
#include "kis_node.h"
#include "commands/kis_node_opacity_command.h"
#include "kis_command_ids.h"
#include "kis_command_utils.h"
#include "kis_keyframe_channel.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_scalar_keyframe_channel.h"

using namespace KisCommandUtils;

KisNodeOpacityCommand::KisNodeOpacityCommand(KisNodeSP node, quint8 newOpacity) :
        KisNodeCommand(kundo2_i18n("Opacity Change"), node)
{
    m_newOpacity = newOpacity;

    KIS_SAFE_ASSERT_RECOVER_RETURN(node->image());

    const int time = node->image()->animationInterface()->currentTime();

    KisKeyframeChannel* channel = m_node->getKeyframeChannel(KisKeyframeChannel::Opacity.id());
    if (channel && !channel->keyframeAt(time)) {
        KisScalarKeyframeChannel* scalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(channel);
        KIS_ASSERT(scalarChannel);
        m_autokey.reset(new SkipFirstRedoWrapper());
        scalarChannel->addScalarKeyframe(time, newOpacity, m_autokey.data());
    }
}

void KisNodeOpacityCommand::redo()
{
    if (!m_oldOpacity) {
        m_oldOpacity = m_node->opacity();
    }

    if (m_autokey) {
        m_autokey->redo();
    }

    m_node->setOpacity(m_newOpacity);
    m_node->setDirty();
}

void KisNodeOpacityCommand::undo()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_oldOpacity);

    m_node->setOpacity(*m_oldOpacity);
    m_node->setDirty();

    if (m_autokey) {
        m_autokey->undo();
    }
}

int KisNodeOpacityCommand::id() const
{
    return KisCommandUtils::ChangeNodeOpacityId;
}

bool KisNodeOpacityCommand::mergeWith(const KUndo2Command *command)
{
    const KisNodeOpacityCommand *other =
        dynamic_cast<const KisNodeOpacityCommand*>(command);

    if (other && other->m_node == m_node) {
        // verify both commands have been executed and they are consecutive
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_oldOpacity);
        KIS_SAFE_ASSERT_RECOVER_NOOP(other->m_oldOpacity);
        KIS_SAFE_ASSERT_RECOVER_NOOP(other->m_oldOpacity && m_newOpacity == *other->m_oldOpacity);

        m_newOpacity = other->m_newOpacity;
        return true;
    }

    return false;
}

bool KisNodeOpacityCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisNodeOpacityCommand *other =
        dynamic_cast<const KisNodeOpacityCommand*>(command);
    if (!other) return false;


    bool otherCreatedKeyframe = other->m_autokey;
    bool weCreatedKeyframe = m_autokey;
    bool canMergeKeyframe = ((otherCreatedKeyframe ^ weCreatedKeyframe) == true) || (!otherCreatedKeyframe && !weCreatedKeyframe);

    return other->m_node == m_node && canMergeKeyframe;
}

bool KisNodeOpacityCommand::canAnnihilateWith(const KUndo2Command *command) const
{
    const KisNodeOpacityCommand *other =
        dynamic_cast<const KisNodeOpacityCommand*>(command);

    if (!other || other->m_node != m_node) {
        return false;
    }

    if (m_autokey || other->m_autokey) {
        return false;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_oldOpacity, false);
    return *m_oldOpacity == other->m_newOpacity;
}
