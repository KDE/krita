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
    KisTransformArgsKeyframe()
        : KisKeyframe()
    {}

    KisTransformArgsKeyframe(const ToolTransformArgs &args)
        : KisKeyframe()
        , args(args)
    {}

    KisKeyframeSP duplicate(KisKeyframeChannel* channel) override {
        return toQShared(new KisTransformArgsKeyframe(args));
    }

    ToolTransformArgs args;
};

KisTransformArgsKeyframeChannel::KisTransformArgsKeyframeChannel(const KoID &id, KisNodeWSP parent, const ToolTransformArgs &initialValue)
    : KisKeyframeChannel(id, parent)
{
    addKeyframe(0);
    KisTransformArgsKeyframe *argsKeyframe = keyframeAt<KisTransformArgsKeyframe>(0).data();
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

KisKeyframeSP KisTransformArgsKeyframeChannel::createKeyframe()
{
    return toQShared( new KisTransformArgsKeyframe() );
}

QPair<int, KisKeyframeSP> KisTransformArgsKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    ToolTransformArgs args;
    args.fromXML(keyframeNode);

    int time = keyframeNode.attribute("time").toInt();
    workaroundBrokenFrameTimeBug(&time);

    KisTransformArgsKeyframe *keyframe = new KisTransformArgsKeyframe(args);

    return QPair<int, KisKeyframeSP>(time, toQShared(keyframe));
}

void KisTransformArgsKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    Q_UNUSED(layerFilename);
    KisTransformArgsKeyframe *key = dynamic_cast<KisTransformArgsKeyframe*>(keyframe.data());
    KIS_ASSERT_RECOVER_RETURN(key);

    key->args.toXML(&keyframeElement);
}

QRect KisTransformArgsKeyframeChannel::affectedRect(int time) const
{
    Q_UNIMPLEMENTED();
    return QRect();
}
