/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KIS_ANIMATION_CYCLE_H
#define KIS_ANIMATION_CYCLE_H

#include "kis_keyframe.h"
#include "kis_time_range.h"

class KisTimeSpan;
class KisFrameSet;
class KisRepeatFrame;

class KRITAIMAGE_EXPORT KisAnimationCycle {

public:
    KisAnimationCycle(KisKeyframeChannel *channel, KisTimeSpan sourceRange);
    KisAnimationCycle(const KisAnimationCycle &cycle, KisTimeSpan newRange);

    KisKeyframeChannel *channel() const;

    /**
     * The full source range repeated by the cycle.
     */
    KisTimeSpan originalRange() const;
    int duration() const;

    void addRepeat(QSharedPointer<KisRepeatFrame> repeat);
    void removeRepeat(QSharedPointer<KisRepeatFrame> repeat);
    const QVector<QWeakPointer<KisRepeatFrame>>& repeats() const;

    QRect affectedRect() const;

    KisFrameSet instancesWithin(KisKeyframeSP original, KisTimeSpan range) const;

private:
    friend class KisKeyframeChannel;

    KisKeyframeChannel *m_channel;
    KisTimeSpan m_range;
    QVector<QWeakPointer<KisRepeatFrame>> m_repeats;
};

class KRITAIMAGE_EXPORT KisRepeatFrame : public KisKeyframeBase
{
public:
    KisRepeatFrame(KisKeyframeChannel *channel, int time, QSharedPointer<KisAnimationCycle> cycle);

    QSharedPointer<KisAnimationCycle> cycle() const;

    QRect affectedRect() const override;

    int getOriginalTimeFor(int time) const;
    KisKeyframeSP getOriginalKeyframeFor(int time) const override;

    /// Returns the earliest time the original frame appears in this repeat, or -1 if it never does.
    int firstInstanceOf(int originalTime) const;

    /** Returns the time at which the previous frame within the repeat appears,
     * or -1 if time is at the first repeated frame.
     * NB: time should be at the start of a repeated frame
     */
    int previousVisibleFrame(int time) const;

    /** Returns the time at which the next frame within the repeat appears,
     * or -1 if time is at the last repeated frame.
     * NB: time should be at the start of a repeated frame
     */
    int nextVisibleFrame(int time) const;

    /**
     * Finds the time of the next keyframe if any.
     * Returns -1 if the cycle continues indefinitely.
     */
    int lastFrame() const;

private:
    QSharedPointer<KisAnimationCycle> m_cycle;
};

#endif
