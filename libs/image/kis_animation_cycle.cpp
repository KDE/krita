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

KisAnimationCycle::KisAnimationCycle(KisKeyframeChannel *channel, KisTimeSpan sourceRange)
    : m_channel(channel)
    , m_range(sourceRange)
{}

KisAnimationCycle::KisAnimationCycle(const KisAnimationCycle &cycle, KisTimeSpan newRange)
    : m_channel(cycle.m_channel)
    , m_range(newRange)
{}

KisKeyframeChannel* KisAnimationCycle::channel() const
{
    return m_channel;
}

KisTimeSpan KisAnimationCycle::originalRange() const
{
    return m_range;
}

int KisAnimationCycle::duration() const
{
    return originalRange().duration();
}

void KisAnimationCycle::addRepeat(QSharedPointer<KisRepeatFrame> repeat)
{
    if (m_repeats.contains(repeat)) return;
    m_repeats.append(repeat);
}

void KisAnimationCycle::removeRepeat(QSharedPointer<KisRepeatFrame> repeat)
{
    m_repeats.removeAll(repeat);
}

const QVector<QWeakPointer<KisRepeatFrame>>& KisAnimationCycle::repeats() const
{
    return m_repeats;
}

KisFrameSet KisAnimationCycle::instancesWithin(KisKeyframeSP original, KisTimeSpan range) const
{
    KisFrameSet frames;

    const int originalTime = original->time();
    const KisKeyframeBaseSP next = original->channel()->nextItem(*original);
    const int frameDuration = (next.isNull()) ? -1 : next->time() - originalTime;
    const int interval = duration();

    int infiniteFrom = frameDuration == -1 ? originalTime : -1;

    QVector<KisTimeSpan> spans;
    Q_FOREACH(const QWeakPointer<KisRepeatFrame> repeatFrame, m_repeats) {
        const QSharedPointer<KisRepeatFrame> repeat = repeatFrame.toStrongRef();
        if (!repeat) continue;

        const int lastOfRepeat = repeat->lastFrame();

        if (range.isEmpty() && lastOfRepeat == -1) {
            infiniteFrom = repeat->firstInstanceOf(originalTime);
        } else {
            const int end = (lastOfRepeat != -1) ? lastOfRepeat : range.end();
            const KisTimeSpan repeatRange{repeat->time(), end};
            const KisTimeSpan relevantRange = range.isEmpty() ? repeatRange : (range & repeatRange);

            int firstInstance = repeat->firstInstanceOf(originalTime);
            if (firstInstance == -1) continue;

            if (firstInstance < relevantRange.start()) {
                firstInstance += interval * ((range.start() - firstInstance) / interval);
            }

            for (int repeatTime = firstInstance; repeatTime <= relevantRange.end(); repeatTime += interval) {
                const bool endsWithinRange = frameDuration != -1 && repeatTime + frameDuration - 1 <= relevantRange.end();
                const int repeatEndTime = endsWithinRange ? (repeatTime + frameDuration - 1) : relevantRange.end();
                spans.append(KisTimeSpan(repeatTime, repeatEndTime));
            }
        }
    }

    frames |= KisFrameSet(spans);

    if (infiniteFrom != -1) {
        frames |= KisFrameSet::infiniteFrom(infiniteFrom);
    } else {
        frames |= KisFrameSet::between(originalTime, originalTime + frameDuration - 1);
    }

    return frames;
}

QRect KisAnimationCycle::affectedRect() const
{
    QRect rect;

    for (auto keyframe : m_channel->itemsWithin(m_range)) {
        const QSharedPointer<KisKeyframe> key = keyframe.dynamicCast<KisKeyframe>();
        KIS_SAFE_ASSERT_RECOVER(key) { continue; }

        rect |= key->affectedRect();
    }

    return rect;
}

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
        ? originalFrameDuration : (m_range.end() - originalTime);

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

        // Skip to the first instance overlapping with the range
        const int firstInstanceEnd = firstInstance + repeatDuration - 1;
        if (firstInstanceEnd < firstFrame) {
            const int repetitionsToSkip = (firstFrame - firstInstanceEnd) / interval;
            firstInstance += interval * repetitionsToSkip;
        }

        // Find the range of each repeat
        for (int repeatTime = firstInstance; repeatTime <= lastFrame; repeatTime += interval) {
            const int repeatStartTime = (repeatTime >= firstFrame) ? repeatTime : firstFrame;
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
