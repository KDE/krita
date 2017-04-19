/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ANIMATION_UTILS_H
#define __KIS_ANIMATION_UTILS_H

#include "kis_types.h"

#include <QModelIndexList>

namespace KisAnimationUtils
{
    void createKeyframeLazy(KisImageSP image, KisNodeSP node, const QString &channel, int time, bool copy);

    struct FrameItem {
        FrameItem() : time(-1) {}
        FrameItem(KisNodeSP _node, const QString &_channel, int _time) : node(_node), channel(_channel), time(_time) {}

        KisNodeSP node;
        QString channel;
        int time;
    };

    typedef QVector<FrameItem> FrameItemList;

    void removeKeyframes(KisImageSP image, const FrameItemList &frames);
    void removeKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int time);

    void sortPointsForSafeMove(QModelIndexList *points, const QPoint &offset);

    KUndo2Command* createMoveKeyframesCommand(const FrameItemList &srcFrames,
                                              const FrameItemList &dstFrames,
                                              bool copy, KUndo2Command *parentCommand = 0);

    void moveKeyframes(KisImageSP image,
                       const FrameItemList &srcFrames,
                       const FrameItemList &dstFrames,
                       bool copy = false);

    void moveKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int srcTime, int dstTime);

    bool supportsContentFrames(KisNodeSP node);

    extern const QString addFrameActionName;
    extern const QString duplicateFrameActionName;
    extern const QString removeFrameActionName;
    extern const QString removeFramesActionName;
    extern const QString lazyFrameCreationActionName;
    extern const QString dropFramesActionName;
    extern const QString showLayerActionName;

    extern const QString newLayerActionName;
    extern const QString addExistingLayerActionName;
    extern const QString removeLayerActionName;

    extern const QString addOpacityKeyframeActionName;
    extern const QString addTransformKeyframeActionName;
    extern const QString removeOpacityKeyframeActionName;
    extern const QString removeTransformKeyframeActionName;
};

#endif /* __KIS_ANIMATION_UTILS_H */
