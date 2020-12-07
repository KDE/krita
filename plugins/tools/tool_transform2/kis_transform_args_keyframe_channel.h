/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TRANSFORM_ARGS_KEYFRAME_CHANNEL_H
#define _KIS_TRANSFORM_ARGS_KEYFRAME_CHANNEL_H

#include "kis_keyframe_channel.h"
#include "kis_keyframe_commands.h"
#include "tool_transform_args.h"
#include "kundo2command.h"


class KisTransformArgsKeyframeChannel : public KisKeyframeChannel
{
public:
    KisTransformArgsKeyframeChannel(const KoID &id, KisNodeWSP parent, const ToolTransformArgs &initialValue);

    ToolTransformArgs &transformArgs(KisKeyframeSP keyframe) const;
    bool hasScalarValue() const;

private:
    friend class KisAnimatedTransformMaskParameters; // TODO: Ugly...

    KisKeyframeSP createKeyframe() override;
    QPair<int, KisKeyframeSP> loadKeyframe(const QDomElement &keyframeNode);
    void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename);

    QRect affectedRect(int time) const override;
};

#endif
