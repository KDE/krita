/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_args_keyframe_channel.h"

//KisTransformArgsKeyframeChannel::KisTransformArgsKeyframeChannel(const KoID &id, KisNodeWSP parent, const ToolTransformArgs &initialValue)
//    : KisKeyframeChannel(id, parent)
//{
//    addKeyframe(0);
//    KisTransformArgsKeyframe *argsKeyframe = keyframeAt<KisTransformArgsKeyframe>(0).data();
//    argsKeyframe->args = initialValue;
//}

//KisKeyframeSP KisTransformArgsKeyframeChannel::createKeyframe()
//{
//    return toQShared( new KisTransformArgsKeyframe() );
//}

//QPair<int, KisKeyframeSP> KisTransformArgsKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
//{
//    ToolTransformArgs args;
//    args.fromXML(keyframeNode);

//    int time = keyframeNode.attribute("time").toInt();
//    workaroundBrokenFrameTimeBug(&time);

//    KisTransformArgsKeyframe *keyframe = new KisTransformArgsKeyframe(args);

//    return QPair<int, KisKeyframeSP>(time, toQShared(keyframe));
//}

//void KisTransformArgsKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
//{
//    Q_UNUSED(layerFilename);
//    KisTransformArgsKeyframe *key = dynamic_cast<KisTransformArgsKeyframe*>(keyframe.data());
//    KIS_ASSERT_RECOVER_RETURN(key);

//    key->args.toXML(&keyframeElement);
//}

//QRect KisTransformArgsKeyframeChannel::affectedRect(int time) const
//{
//    Q_UNIMPLEMENTED();
//    return QRect();
//}
