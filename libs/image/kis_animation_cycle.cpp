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

#include "kis_animation_cycle.h"
#include <kis_pointer_utils.h>
#include "kis_time_range.h"
#include "kis_keyframe_channel.h"

KisRepeatFrame::KisRepeatFrame(KisKeyframeChannel *channel, int time, KisTimeSpan sourceRange)
        : KisKeyframeBase(channel, time)
        , m_range(sourceRange)
{}

KisRepeatFrame::KisRepeatFrame(const KisRepeatFrame &rhs, KisTimeSpan newRange)
    : KisRepeatFrame(rhs.channel(), rhs.time(), newRange)
{}

KisRepeatFrame::KisRepeatFrame(const KisRepeatFrame &rhs, KisKeyframeChannel *newChannel)
    : KisRepeatFrame(newChannel, rhs.time(), rhs.m_range)
{}

QRect KisRepeatFrame::affectedRect() const
{
    QRect rect;

    for (auto keyframe : channel()->itemsWithin(m_range)) {
        const QSharedPointer<KisKeyframe> key = keyframe.dynamicCast<KisKeyframe>();
        KIS_SAFE_ASSERT_RECOVER(key) { continue; }

        rect |= key->affectedRect();
    }

    return rect;
}

KisTimeSpan KisRepeatFrame::sourceRange() const
{
    return m_range;
}

int KisRepeatFrame::getOriginalTimeFor(int time) const
{
    KisTimeSpan originalRange = m_range;
    const int timeWithinCycle = (time - this->time()) % originalRange.duration();
    return originalRange.start() + timeWithinCycle;
}

KisKeyframeSP KisRepeatFrame::getOriginalKeyframeFor(int time) const
{
    return channel()->activeKeyframeAt(getOriginalTimeFor(time));
}

int KisRepeatFrame::firstInstanceOf(int originalTime) const
{
    const int timeWithinCycle = originalTime - m_range.start();

    if (timeWithinCycle < this->duration()) return -1;

    return this->time() + timeWithinCycle;
}

KisFrameSet KisRepeatFrame::instancesWithin(KisKeyframeSP original, KisTimeSpan range) const
{
    const int originalTime = original->time();
    const int interval = m_range.duration();
    const int lastOfCycle = lastFrame();

    const int originalFrameDuration = original->duration();
    const int repeatDuration = (originalTime + originalFrameDuration < m_range.end())
        ? originalFrameDuration : (m_range.end() - originalTime + 1);

    QVector<KisTimeSpan> spans;

    int firstInstance = firstInstanceOf(originalTime);

    if (firstInstance == -1) {
      return {};
    } else if (range.isEmpty() && lastOfCycle == -1) {
        return KisFrameSet::infiniteFrom(firstInstance);
    } else {
        // Determine the relevant range to consider, ie. the overlap of repeat and the given range (if any)
        const int firstFrame = KisTime::max(range.start(), time());
        const int lastFrame = KisTime::min(lastOfCycle, range.end());

        // Skip past the instances before the range
        const int firstInstanceEnd = firstInstance + repeatDuration - 1;
        if (firstInstanceEnd < firstFrame) {
            const int repetitionsToSkip = 1 + (firstFrame - firstInstanceEnd - 1) / interval; // effectively: ceil((firstFrame - firstInstanceEnd) / interval)
            firstInstance += interval * repetitionsToSkip;
        }

        // Find the range of each repeat
        for (int repeatTime = firstInstance; repeatTime <= lastFrame; repeatTime += interval) {
            const int repeatStartTime = std::max(repeatTime, firstFrame);
            const bool endsWithinRange = repeatDuration != -1 && repeatTime + repeatDuration - 1 <= lastFrame;
            const int repeatEndTime = endsWithinRange ? (repeatTime + repeatDuration - 1) : lastFrame;
            spans.append(KisTimeSpan(repeatStartTime, repeatEndTime));
        }
    }

    return KisFrameSet(spans);
}

int KisRepeatFrame::previousVisibleFrame(int time) const
{
    if (time <= this->time()) return -1;

    const int earlierOriginalTime = getOriginalTimeFor(time - 1);

    int originalStart, originalEnd;
    channel()->activeKeyframeRange(earlierOriginalTime, &originalStart, &originalEnd);
    if (originalEnd == -1) return -1;

    const int durationOfOriginalKeyframe = originalEnd + 1 - originalStart;
    return time - durationOfOriginalKeyframe;
}

int KisRepeatFrame::nextVisibleFrame(int time) const
{
    const int originalTime = getOriginalTimeFor(time);
    int originalStart, originalEnd;
    channel()->activeKeyframeRange(originalTime, &originalStart, &originalEnd);
    if (originalEnd == -1) return -1;

    const int durationOfOriginalKeyframe = originalEnd + 1 - originalStart;
    const int nextFrameTime = time + durationOfOriginalKeyframe;

    const int endTime = lastFrame();
    return (endTime == -1 || nextFrameTime < endTime) ? nextFrameTime : -1;
}

int KisRepeatFrame::lastFrame() const
{
    const KisKeyframeBaseSP next = channel()->nextItem(*this);
    return next ? next->time() - 1 : -1;
}
