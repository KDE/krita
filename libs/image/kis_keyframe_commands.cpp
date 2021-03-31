/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_keyframe_commands.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_signals_blocker.h"
#include "kis_image.h"

KisInsertKeyframeCommand::KisInsertKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeSP keyframe, KUndo2Command *parentCmd)
    : KUndo2Command(parentCmd),
      m_channel(channel),
      m_time(time),
      m_keyframe(keyframe)
{
    m_overwritten = m_channel->keyframeAt(m_time);
}

void KisInsertKeyframeCommand::redo()
{
    m_channel->insertKeyframe(m_time, m_keyframe);
}

void KisInsertKeyframeCommand::undo()
{
    m_channel->removeKeyframe(m_time);

    if (m_overwritten) {
        m_channel->insertKeyframe(m_time, m_overwritten);
    }
}


KisRemoveKeyframeCommand::KisRemoveKeyframeCommand(KisKeyframeChannel *channel, int time, KUndo2Command *parentCmd)
    : KUndo2Command(parentCmd),
      m_channel(channel),
      m_time(time)
{
    m_cached = channel->keyframeAt(time);
}

void KisRemoveKeyframeCommand::redo()
{
    m_channel->removeKeyframe(m_time);
}

void KisRemoveKeyframeCommand::undo()
{
    m_channel->insertKeyframe(m_time, m_cached);
}

KisScalarKeyframeUpdateCommand::KisScalarKeyframeUpdateCommand(KisScalarKeyframe *keyframe, qreal value, KisScalarKeyframe::InterpolationMode interpolationMode, KisScalarKeyframe::TangentsMode tangentMode, QPointF tangentLeft, QPointF tangentRight, KUndo2Command *parentCmd)
    : KUndo2Command(parentCmd),
      keyframe(keyframe),
      cachedValue(keyframe->value(), value),
      cachedInterpolationMode(keyframe->interpolationMode(), interpolationMode),
      cachedTangentsMode(keyframe->tangentsMode(), tangentMode),
      cachedTangentLeft(keyframe->leftTangent(), tangentLeft),
      cachedTangentRight(keyframe->rightTangent(), tangentRight)
{}

void KisScalarKeyframeUpdateCommand::redo()
{
    KIS_ASSERT(keyframe);

    QSharedPointer<ScalarKeyframeLimits> limits = keyframe->m_channelLimits.toStrongRef();
    if (limits) {
        keyframe->m_value = limits->clamp(cachedValue.second);
    } else {
        keyframe->m_value = cachedValue.second;
    }

    keyframe->m_interpolationMode = cachedInterpolationMode.second;
    keyframe->m_tangentsMode = cachedTangentsMode.second;
    keyframe->m_leftTangent = cachedTangentLeft.second;
    keyframe->m_rightTangent = cachedTangentRight.second;

    keyframe->sigChanged(keyframe);
}

void KisScalarKeyframeUpdateCommand::undo()
{
    KIS_ASSERT(keyframe);

    QSharedPointer<ScalarKeyframeLimits> limits = keyframe->m_channelLimits.toStrongRef();
    if (limits) {
        keyframe->m_value = limits->clamp(cachedValue.first);
    } else {
        keyframe->m_value = cachedValue.first;
    }

    keyframe->m_interpolationMode = cachedInterpolationMode.first;
    keyframe->m_tangentsMode = cachedTangentsMode.first;
    keyframe->m_leftTangent = cachedTangentLeft.first;
    keyframe->m_rightTangent = cachedTangentRight.first;

    keyframe->sigChanged(keyframe);
}
