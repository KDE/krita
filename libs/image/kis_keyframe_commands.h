/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisScalarKeyframeUpdateCommand(KisScalarKeyframe* keyframe,
                                   qreal value,
                                   KisScalarKeyframe::InterpolationMode interpolationMode,
                                   KisScalarKeyframe::TangentsMode tangentMode,
                                   QPointF tangentLeft,
                                   QPointF tangentRight,
                                   KUndo2Command *parentCmd);

    KisScalarKeyframeUpdateCommand(KisScalarKeyframe* keyframe,
                                   qreal value,
                                   KUndo2Command *parentCmd)
        : KisScalarKeyframeUpdateCommand(keyframe, value, keyframe->m_interpolationMode,
                                         keyframe->m_tangentsMode, keyframe->m_leftTangent,
                                         keyframe->m_rightTangent, parentCmd) {};

    KisScalarKeyframeUpdateCommand(KisScalarKeyframe* keyframe,
                                   KisScalarKeyframe::InterpolationMode interpMode,
                                   KUndo2Command *parentCmd)
        : KisScalarKeyframeUpdateCommand(keyframe, keyframe->m_value, interpMode,
                                         keyframe->m_tangentsMode, keyframe->m_leftTangent,
                                         keyframe->m_rightTangent, parentCmd) {};

    KisScalarKeyframeUpdateCommand(KisScalarKeyframe* keyframe,
                                   KisScalarKeyframe::TangentsMode tangentMode,
                                   KUndo2Command *parentCmd)
        : KisScalarKeyframeUpdateCommand(keyframe, keyframe->m_value, keyframe->m_interpolationMode,
                                         tangentMode, keyframe->m_leftTangent,
                                         keyframe->m_rightTangent, parentCmd) {};

    KisScalarKeyframeUpdateCommand(KisScalarKeyframe* keyframe,
                                   QPointF tangentLeft,
                                   QPointF tangentRight,
                                   KUndo2Command *parentCmd)
        : KisScalarKeyframeUpdateCommand(keyframe, keyframe->m_value, keyframe->m_interpolationMode,
                                         keyframe->m_tangentsMode, tangentLeft,
                                         tangentRight, parentCmd) {};

    void redo() override;
    void undo() override;

    template<typename T>
    using UndoStore = QPair<T, T>;

private:
    KisScalarKeyframe* keyframe;
    UndoStore<qreal> cachedValue;
    UndoStore<KisScalarKeyframe::InterpolationMode> cachedInterpolationMode;
    UndoStore<KisScalarKeyframe::TangentsMode> cachedTangentsMode;
    UndoStore<QPointF> cachedTangentLeft;
    UndoStore<QPointF> cachedTangentRight;

};


#endif
