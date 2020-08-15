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

KisScalarKeyframeUpdateCommand::KisScalarKeyframeUpdateCommand(KisScalarKeyframe *keyframe, KUndo2Command *parentCmd)
    : KUndo2Command(parentCmd),
      keyframe(keyframe),
      cachedValue(keyframe->value()),
      cachedInterpolationMode(keyframe->interpolationMode()),
      cachedTangentsMode(keyframe->tangentsMode()),
      cachedTangentLeft(keyframe->leftTangent()),
      cachedTangentRight(keyframe->rightTangent())
{}

void KisScalarKeyframeUpdateCommand::redo()
{
    //Note -- cached values are swapped, so undo / redo can be the same.
    KisScalarKeyframeUpdateCommand::undo();
}

void KisScalarKeyframeUpdateCommand::undo()
{
    const qreal value = keyframe->value();
    const KisScalarKeyframe::InterpolationMode interpolationMode = keyframe->interpolationMode();
    const KisScalarKeyframe::TangentsMode tangentsMode = keyframe->tangentsMode();
    const QPointF leftTangent = keyframe->leftTangent();
    const QPointF rightTangent = keyframe->rightTangent();

    {
        // We block the signals for `keyframe` so that 5 signals aren't emitted on undo/redo..
        KisSignalsBlocker blocker(keyframe);
        keyframe->setValue(cachedValue);
        keyframe->setInterpolationMode(cachedInterpolationMode);
        keyframe->setTangentsMode(cachedTangentsMode);
        keyframe->setInterpolationTangents(cachedTangentLeft, cachedTangentRight);
    }

    // ..Because of the prior signal blocking, we need to manually emit the sigChanged signal once.
    keyframe->sigChanged(keyframe);

    cachedValue = value;
    cachedInterpolationMode = interpolationMode;
    cachedTangentsMode = tangentsMode;
    cachedTangentLeft = leftTangent;
    cachedTangentRight = rightTangent;
}
