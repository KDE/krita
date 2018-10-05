/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_transform_args_keyframe_channel.h"

struct KisTransformArgsKeyframe : public KisKeyframe
{
    KisTransformArgsKeyframe(KisTransformArgsKeyframeChannel *channel, int time)
        : KisKeyframe(channel, time)
    {}

    KisTransformArgsKeyframe(KisTransformArgsKeyframeChannel *channel, int time, const ToolTransformArgs &args)
        : KisKeyframe(channel, time)
        , args(args)
    {}

    KisTransformArgsKeyframe(const KisTransformArgsKeyframe *rhs, KisKeyframeChannel *channel)
        : KisKeyframe(rhs, channel)
        , args(rhs->args)
    {}

    ToolTransformArgs args;

    KisKeyframeSP cloneFor(KisKeyframeChannel *channel) const override
    {
        KisTransformArgsKeyframeChannel *argsChannel = dynamic_cast<KisTransformArgsKeyframeChannel*>(channel);
        Q_ASSERT(argsChannel);
        return toQShared(new KisTransformArgsKeyframe(this, channel));
    }

    QRect affectedRect() const override
    {
        // TODO
        return QRect();
    }
};

KisTransformArgsKeyframeChannel::AddKeyframeCommand::AddKeyframeCommand(KisTransformArgsKeyframeChannel *channel, int time, const ToolTransformArgs &args, KUndo2Command *parentCommand)
    : KisReplaceKeyframeCommand(channel, time, toQShared(new KisTransformArgsKeyframe(channel, time, args)), parentCommand)
{
}

KisTransformArgsKeyframeChannel::KisTransformArgsKeyframeChannel(const KoID &id, KisDefaultBoundsBaseSP defaultBounds, const ToolTransformArgs &initialValue)
    : KisKeyframeChannel(id, defaultBounds)
{
    KisKeyframeSP keyframe = addKeyframe(0);
    KisTransformArgsKeyframe *argsKeyframe = dynamic_cast<KisTransformArgsKeyframe*>(keyframe.data());
    argsKeyframe->args = initialValue;
}

ToolTransformArgs &KisTransformArgsKeyframeChannel::transformArgs(KisKeyframeSP keyframe) const
{
    KisTransformArgsKeyframe *key = dynamic_cast<KisTransformArgsKeyframe*>(keyframe.data());
    Q_ASSERT(key != 0);
    return key->args;
}

bool KisTransformArgsKeyframeChannel::hasScalarValue() const
{
    return false;
}

KisKeyframeSP KisTransformArgsKeyframeChannel::createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    Q_UNUSED(parentCommand);
    KisTransformArgsKeyframe *srcKey = dynamic_cast<KisTransformArgsKeyframe*>(copySrc.data());
    KisTransformArgsKeyframe *newKey;

    if (srcKey) {
        newKey = new KisTransformArgsKeyframe(this, time, srcKey->args);
    } else {
        newKey = new KisTransformArgsKeyframe(this, time);
    }

    return toQShared(newKey);
}

void KisTransformArgsKeyframeChannel::destroyKeyframe(KisKeyframeSP, KUndo2Command*)
{}

void KisTransformArgsKeyframeChannel::uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame)
{
    Q_UNUSED(srcChannel);
    Q_UNUSED(srcTime);
    Q_UNUSED(dstFrame);
}

KisKeyframeSP KisTransformArgsKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    ToolTransformArgs args;
    args.fromXML(keyframeNode);

    int time = keyframeNode.attribute("time").toInt();
    workaroundBrokenFrameTimeBug(&time);

    KisTransformArgsKeyframe *keyframe = new KisTransformArgsKeyframe(this, time, args);

    return toQShared(keyframe);
}

void KisTransformArgsKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    Q_UNUSED(layerFilename);
    KisTransformArgsKeyframe *key = dynamic_cast<KisTransformArgsKeyframe*>(keyframe.data());
    KIS_ASSERT_RECOVER_RETURN(key);

    key->args.toXML(&keyframeElement);
}
