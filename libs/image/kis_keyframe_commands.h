/*
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
#include "kritaimage_export.h"

namespace KisKeyframeCommands
{
    struct ValidationResult {
        enum Status {
            Valid,
            RepeatKeyframeWithinCycleDefinition,
            KeyframesFromDifferentChannels,
            MultipleDestinations
        };

        ValidationResult(Status error) // NOLINT(google-explicit-constructor)
            : status(error)
            , command(nullptr)
        {}

        explicit ValidationResult(KUndo2Command *command)
            : status(Valid)
            , command(command)
        {
            KIS_SAFE_ASSERT_RECOVER_NOOP(command);
        }

        bool isValid() const {
            return status == Valid;
        }

        bool operator==(const ValidationResult &rhs) const
        {
            return status == rhs.status && command == rhs.command;
        }

        bool operator!=(const ValidationResult &rhs) const
        {
            return !(rhs == *this);
        }

        bool operator==(const Status &rhs) const
        {
            return status == rhs;
        }

        Status status;
        KUndo2Command *command;
    };

    struct KRITAIMAGE_EXPORT KeyframeMove
    {
        KisKeyframeBaseSP keyframe;
        int oldTime, newTime;

        KeyframeMove() = default;
        KeyframeMove(KisKeyframeBaseSP keyframe, int newTime);
    };

    /**
     * Returns either a new command for operations needed to move the keyframes or null if the operation is invalid against the current state of the channel
     */
    KRITAIMAGE_EXPORT ValidationResult tryMoveKeyframes(KisKeyframeChannel *channel, QVector<KeyframeMove> moves, KUndo2Command *parentCommand);
}

class KRITAIMAGE_EXPORT KisReplaceKeyframeCommand : public KUndo2Command
{
public:
    KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel *m_channel;
    int m_time;
    KisKeyframeBaseSP m_keyframe;
    KisKeyframeBaseSP m_existingKeyframe;
};

class KRITAIMAGE_EXPORT KisMoveFrameCommand : public KUndo2Command
{
public:
    KisMoveFrameCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP keyframe, int oldTime, int newTime, KUndo2Command *parentCommand);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel *m_channel;
    KisKeyframeBaseSP m_keyframe;
    int m_oldTime;
    int m_newTime;
};

class KRITAIMAGE_EXPORT KisSwapFramesCommand : public KUndo2Command
{
public:
    KisSwapFramesCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP lhsFrame, KisKeyframeBaseSP rhsFrame, KUndo2Command *parentCommand);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel *m_channel;
    KisKeyframeBaseSP m_lhsFrame;
    KisKeyframeBaseSP m_rhsFrame;
};

class KRITAIMAGE_EXPORT KisDefineCycleCommand : public KUndo2Command
{
public:
    KisDefineCycleCommand(QSharedPointer<KisAnimationCycle> oldCycle, QSharedPointer<KisAnimationCycle> newCycle, KUndo2Command *parentCommand);

    QSharedPointer<KisAnimationCycle> cycle() const;

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel *m_channel;
    QSharedPointer<KisAnimationCycle> m_oldCycle;
    QSharedPointer<KisAnimationCycle> m_newCycle;
};

#endif
