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

#ifndef KIS_KEYFRAME_COMMANDS_H
#define KIS_KEYFRAME_COMMANDS_H

#include "kis_keyframe_channel.h"
#include "kundo2command.h"
#include "kis_scalar_keyframe_channel.h"
#include "kritaimage_export.h"


class KisInsertKeyframeCommand : public KUndo2Command
{
public:
    KisInsertKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeSP keyframe, KUndo2Command *parentCmd);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel* m_channel;
    int m_time;
    KisKeyframeSP m_keyframe;

    KisKeyframeSP m_overwritten = nullptr;
};


class KisRemoveKeyframeCommand : public KUndo2Command
{
public:
    KisRemoveKeyframeCommand(KisKeyframeChannel *channel, int time, KUndo2Command* parentCmd);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel* m_channel;
    int m_time;

    KisKeyframeSP m_cached;
};


class KisScalarKeyframeUpdateCommand : public KUndo2Command
{
public:
    KisScalarKeyframeUpdateCommand(KisScalarKeyframe* keyframe, KUndo2Command *parentCmd);

    void redo() override;
    void undo() override;

private:
    KisScalarKeyframe* keyframe;
    qreal cachedValue;
    KisScalarKeyframe::InterpolationMode cachedInterpolationMode;
    KisScalarKeyframe::TangentsMode cachedTangentsMode;
    QPointF cachedTangentLeft;
    QPointF cachedTangentRight;

};


#endif
