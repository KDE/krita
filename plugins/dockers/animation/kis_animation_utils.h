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

#include <boost/operators.hpp>
#include <QModelIndexList>

#include <kritaanimationdocker_export.h>


namespace KisAnimationUtils
{
    KUndo2Command* createKeyframeCommand(KisImageSP image, KisNodeSP node, const QString &channelId, int time, bool copy, KUndo2Command *parentCommand = 0);
    void createKeyframeLazy(KisImageSP image, KisNodeSP node, const QString &channel, int time, bool copy);

    struct KRITAANIMATIONDOCKER_EXPORT FrameItem : public boost::equality_comparable<FrameItem>
    {
        FrameItem() : time(-1) {}
        FrameItem(KisNodeSP _node, const QString &_channel, int _time) : node(_node), channel(_channel), time(_time) {}

        bool operator==(const FrameItem &rhs) const {
            return rhs.node == node && rhs.channel == channel && rhs.time == time;
        }

        KisNodeSP node;
        QString channel;
        int time;
    };

    KRITAANIMATIONDOCKER_EXPORT QDebug operator<<(QDebug dbg, const FrameItem &item);

    inline uint qHash(const FrameItem &item)
    {
        return ::qHash(item.node.data()) + ::qHash(item.channel) + ::qHash(item.time);
    }


    typedef QVector<FrameItem> FrameItemList;
    typedef std::pair<FrameItem, FrameItem> FrameMovePair;
    typedef QVector<FrameMovePair> FrameMovePairList;


    void removeKeyframes(KisImageSP image, const FrameItemList &frames);
    void removeKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int time);

    void sortPointsForSafeMove(QModelIndexList *points, const QPoint &offset);

    KUndo2Command* createMoveKeyframesCommand(const FrameItemList &srcFrames, const FrameItemList &dstFrames,
                                              bool copy, bool moveEmpty, KUndo2Command *parentCommand = 0);


    /**
     * @brief implements safe moves of the frames (even if there are cycling move dependencies)
     * @param movePairs the jobs for the moves
     * @param copy shows if the frames should be copied or not
     * @param moveEmpty allows an empty frame to replace a populated one
     * @param parentCommand the command that should be a parent of the created command
     * @return a created undo command
     */
    KRITAANIMATIONDOCKER_EXPORT
    KUndo2Command* createMoveKeyframesCommand(const FrameMovePairList &movePairs,
                                              bool copy, bool moveEmptyFrames, KUndo2Command *parentCommand = 0);

    bool supportsContentFrames(KisNodeSP node);

    extern const QString lazyFrameCreationActionName;
    extern const QString dropFramesActionName;

    extern const QString newLayerActionName;
    extern const QString addExistingLayerActionName;
    extern const QString removeLayerActionName;

    extern const QString addOpacityKeyframeActionName;
    extern const QString addTransformKeyframeActionName;
    extern const QString removeOpacityKeyframeActionName;
    extern const QString removeTransformKeyframeActionName;
};


#endif /* __KIS_ANIMATION_UTILS_H */
