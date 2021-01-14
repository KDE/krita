/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ANIMATION_UTILS_H
#define __KIS_ANIMATION_UTILS_H

#include "kis_types.h"

#include <boost/operators.hpp>
#include <QModelIndexList>

#include <kritaanimationdocker_export.h>


namespace KisAnimUtils
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
    void resetChannels(KisImageSP image, KisNodeSP node, const QList<QString> &channelIDs);
    void resetChannel(KisImageSP image, KisNodeSP node, const QString &channelID);

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

    KUndo2Command* createCloneKeyframesCommand(const FrameMovePairList &srcDstPairs,
                                               KUndo2Command *parentCommand);

    void makeClonesUnique(KisImageSP image, const FrameItemList &frames);

    bool supportsContentFrames(KisNodeSP node);

    extern const QString lazyFrameCreationActionName;
    extern const QString dropFramesActionName;

    extern const QString newLayerActionName;
    extern const QString pinExistingLayerActionName;
    extern const QString removeLayerActionName;

    extern const QString addOpacityKeyframeActionName;
    extern const QString addTransformKeyframeActionName;
    extern const QString removeOpacityKeyframeActionName;
    extern const QString removeTransformKeyframeActionName;
};


#endif /* __KIS_ANIMATION_UTILS_H */
