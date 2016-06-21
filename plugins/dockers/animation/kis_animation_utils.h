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


namespace KisAnimationUtils
{
    bool createKeyframeLazy(KisImageSP image, KisNodeSP node, const QString &channel, int time, bool copy);

    struct FrameItem {
        FrameItem() : time(-1) {}
        FrameItem(KisNodeSP _node, const QString &_channel, int _time) : node(_node), channel(_channel), time(_time) {}

        KisNodeSP node;
        QString channel;
        int time;
    };

    typedef QVector<FrameItem> FrameItemList;

    bool removeKeyframes(KisImageSP image, const FrameItemList &frames);
    bool removeKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int time);

    void sortPointsForSafeMove(QVector<QPoint> *points, const QPoint &offset);
    bool moveKeyframes(KisImageSP image,
                       const FrameItemList &srcFrames,
                       const FrameItemList &dstFrames,
                       bool copy = false);
    bool moveKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int srcTime, int dstTime);


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
