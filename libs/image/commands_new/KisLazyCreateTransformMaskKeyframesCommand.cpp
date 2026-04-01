/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisLazyCreateTransformMaskKeyframesCommand.h"

#include "kis_transform_mask.h"
#include "kis_scalar_keyframe_channel.h"

KisLazyCreateTransformMaskKeyframesCommand::KisLazyCreateTransformMaskKeyframesCommand(KisTransformMaskSP mask, KUndo2Command *parent)
    : KisCommandUtils::AggregateCommand(parent)
    , m_mask(mask)
{
}

bool KisLazyCreateTransformMaskKeyframesCommand::maskHasAnimation(KisTransformMaskSP mask)
{
    const QVector<QString> ids = {KisKeyframeChannel::PositionX.id(),
                                  KisKeyframeChannel::PositionY.id(),
                                  KisKeyframeChannel::ScaleX.id(),
                                  KisKeyframeChannel::ScaleY.id(),
                                  KisKeyframeChannel::ShearX.id(),
                                  KisKeyframeChannel::ShearY.id(),
                                  KisKeyframeChannel::RotationX.id(),
                                  KisKeyframeChannel::RotationY.id(),
                                  KisKeyframeChannel::RotationZ.id()};

    Q_FOREACH (const QString &id, ids) {
        if (mask->getKeyframeChannel(id)) return true;
    }

    return false;
}

void KisLazyCreateTransformMaskKeyframesCommand::populateChildCommands()
{
    std::unique_ptr<KUndo2Command> parentCommand(new KUndo2Command);

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_mask->parent());

    const int time = m_mask->parent()->original()->defaultBounds()->currentTime();

    auto addKeyframe = [this, time] (const KoID &channelId, KUndo2Command *parentCommand)
    {
        KisScalarKeyframeChannel *channel = dynamic_cast<KisScalarKeyframeChannel*>(m_mask->getKeyframeChannel(channelId.id()));
        KIS_SAFE_ASSERT_RECOVER_RETURN(channel);

        if (!channel->keyframeAt(time)) {
            channel->addScalarKeyframe(time, channel->valueAt(time), parentCommand);
        }
    };

    addKeyframe(KisKeyframeChannel::PositionX, parentCommand.get());
    addKeyframe(KisKeyframeChannel::PositionY, parentCommand.get());

    addKeyframe(KisKeyframeChannel::ScaleX, parentCommand.get());
    addKeyframe(KisKeyframeChannel::ScaleY, parentCommand.get());

    addKeyframe(KisKeyframeChannel::ShearX, parentCommand.get());
    addKeyframe(KisKeyframeChannel::ShearY, parentCommand.get());

    addKeyframe(KisKeyframeChannel::RotationX, parentCommand.get());
    addKeyframe(KisKeyframeChannel::RotationY, parentCommand.get());
    addKeyframe(KisKeyframeChannel::RotationZ, parentCommand.get());

    addCommand(parentCommand.release());
}
