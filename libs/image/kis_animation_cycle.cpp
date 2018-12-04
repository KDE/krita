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

KisAnimationCycle::KisAnimationCycle(KisKeyframeChannel *channel, KisKeyframeSP firstKeyframe, KisKeyframeSP lastKeyframe)
        : KisKeyframeBase(channel, firstKeyframe->time())
        , m_firstSourceKeyframe(firstKeyframe)
        , m_lastSourceKeyframe(lastKeyframe)
{}

KisKeyframeSP KisAnimationCycle::firstSourceKeyframe() const
{
    return m_firstSourceKeyframe;
}

KisKeyframeSP KisAnimationCycle::lastSourceKeyframe() const
{
    return m_lastSourceKeyframe;
}

KisTimeSpan KisAnimationCycle::originalRange() const
{
    const KisKeyframeBaseSP firstAfterCycle = m_lastSourceKeyframe->channel()->nextItem(*m_lastSourceKeyframe);

    KisTimeSpan range;
    if (firstAfterCycle.isNull()) {
        // TODO: semantics of repeat definition without a terminating keyframe?
        range = KisTimeSpan(m_firstSourceKeyframe->time(), m_lastSourceKeyframe->time());
    } else {
        range = KisTimeSpan(m_firstSourceKeyframe->time(), firstAfterCycle->time() - 1);
    }
    return range;
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
            auto repeat = repeatFrame.toStrongRef();
            if (!repeat) continue;

            const int endTime = repeat->end();

            if (range.isEmpty()) {
                if (endTime != -1) {
                    range = KisTimeSpan(0, endTime - 1);
                } else {
                    infiniteFrom = repeat->firstInstanceOf(originalTime);
                    continue;
                }
            }

            KisTimeSpan repeatRange = (endTime != -1) ? range & KisTimeSpan(repeat->time(), endTime) : range.truncateRight(repeat->time());
            int firstInstance = repeat->firstInstanceOf(originalTime);
            if (firstInstance == -1) continue;

            if (firstInstance < repeatRange.start()) {
                firstInstance += interval * ((range.start() - firstInstance) / interval);
            }

            for (int repeatTime = firstInstance; repeatTime <= repeatRange.end(); repeatTime += interval) {
                bool endsWithinRange = frameDuration != -1 && repeatTime + frameDuration - 1 <= repeatRange.end();
                const int repeatEndTime = endsWithinRange ? (repeatTime + frameDuration - 1) : repeatRange.end();
                spans.append(KisTimeSpan(repeatTime, repeatEndTime));
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

    KisKeyframeSP keyframe = m_firstSourceKeyframe;
    do {
        rect |= keyframe->affectedRect();
        keyframe = channel()->nextKeyframe(keyframe);
    } while (keyframe && keyframe != m_lastSourceKeyframe);

    return rect;
}

KisKeyframeSP KisAnimationCycle::getOriginalKeyframeFor(int time) const
{
    return channel()->activeKeyframeAt(time);
}

KisRepeatFrame::KisRepeatFrame(KisKeyframeChannel *channel, int time, QSharedPointer<KisAnimationCycle> cycle)
        : KisKeyframeBase(channel, time)
        , m_cycle(cycle)
{}

QSharedPointer<KisAnimationCycle> KisRepeatFrame::cycle() const
{
    return m_cycle;
}

QRect KisRepeatFrame::affectedRect() const
{
    KisKeyframeSP keyframe = m_cycle->firstSourceKeyframe();

    QRect rect;
    while (!keyframe.isNull() && keyframe->time() <= m_cycle->lastSourceKeyframe()->time()) {
        rect |= keyframe->affectedRect();
        keyframe = channel()->nextKeyframe(keyframe);
    }

    return rect;
}

int KisRepeatFrame::getOriginalTimeFor(int time) const
{
    KisTimeSpan originalRange = m_cycle->originalRange();
    int timeWithinCycle = (time - this->time()) % originalRange.duration();
    return m_cycle->firstSourceKeyframe()->time() + timeWithinCycle;
}

KisKeyframeSP KisRepeatFrame::getOriginalKeyframeFor(int time) const
{
    return channel()->activeKeyframeAt(getOriginalTimeFor(time));
}

int KisRepeatFrame::firstInstanceOf(int originalTime) const
{
    KisTimeSpan originalRange = m_cycle->originalRange();
    const int timeWithinCycle = originalTime - originalRange.start();

    const int first = this->time() + timeWithinCycle;

    const KisKeyframeSP next = channel()->nextKeyframe(*this);
    if (next && next->time() < first) return -1;

    return first;
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

    const int endTime = end();
    return (endTime == -1 || nextFrameTime < endTime) ? nextFrameTime : -1;
}

int KisRepeatFrame::end() const
{
    const KisKeyframeBaseSP next = channel()->nextItem(*this);
    return next ? next->time() - 1 : -1;
}
