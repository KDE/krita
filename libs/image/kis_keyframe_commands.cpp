/*
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_keyframe_commands.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_signals_blocker.h"

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
    if (!keyframe)
        return;

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
    if (!keyframe)
        return;

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

KisMoveKeyframeInternalCommand::KisMoveKeyframeInternalCommand(KisKeyframeChannel *channel, int srcTime, int dstTime, KUndo2Command *parentCmd)
    :   KUndo2Command(parentCmd)
    , m_channel(channel)
    , m_srcTime(srcTime)
    , m_dstTime(dstTime)
{}

void KisMoveKeyframeInternalCommand::redo()
{
    m_channel->moveKeyframe(m_srcTime, m_dstTime);
}

void KisMoveKeyframeInternalCommand::undo()
{
    m_channel->moveKeyframe(m_dstTime, m_srcTime);
}

KisSwapKeyframesInternalCommand::KisSwapKeyframesInternalCommand(KisKeyframeChannel *channel, int timeA, int timeB, KUndo2Command *parentCmd)
    : KUndo2Command(parentCmd)
    , m_channel(channel)
    , m_timeA(timeA)
    , m_timeB(timeB)
{

}

void KisSwapKeyframesInternalCommand::redo()
{
    m_channel->swapKeyframes(m_timeA, m_timeB);
}

void KisSwapKeyframesInternalCommand::undo()
{
    m_channel->swapKeyframes(m_timeB, m_timeA);
}
