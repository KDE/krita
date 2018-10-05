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
#include "kis_time_range.h"
#include "kis_keyframe_channel.h"

KisAnimationCycle::KisAnimationCycle(KisKeyframeSP firstKeyframe, KisKeyframeSP lastKeyframe)
        : m_firstSourceKeyframe(firstKeyframe)
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
    const KisKeyframeSP firstAfterCycle = m_lastSourceKeyframe->channel()->nextKeyframe(m_lastSourceKeyframe);

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
    const KisKeyframeSP next = original->channel()->nextKeyframe(original);
    const int frameDuration = (next.isNull()) ? -1 : next->time() - originalTime;
    const int interval = duration();

    int infiniteFrom = frameDuration == -1 ? originalTime : -1;

    QVector<KisTimeSpan> spans;
    Q_FOREACH(const QWeakPointer<KisRepeatFrame> repeatFrame, m_repeats) {
            auto repeat = repeatFrame.toStrongRef();
            if (!repeat) continue;

            KisKeyframeSP terminatingKey = repeat->channel()->nextKeyframe(*repeat);

            if (range.isEmpty()) {
                if (terminatingKey) {
                    range = KisTimeSpan(0, terminatingKey->time() -1);
                } else {
                    infiniteFrom = repeat->firstInstanceOf(originalTime);
                    continue;
                }
            }

            KisTimeSpan repeatRange = terminatingKey ? range.truncateLeft(terminatingKey->time() - 1) : range;
            int firstInstance = repeat->firstInstanceOf(originalTime);
            if (firstInstance < repeatRange.start()) firstInstance += (range.start() - firstInstance) / interval * interval;

            for (int repeatTime = firstInstance; repeatTime <= repeatRange.end(); repeatTime += interval) {
                const int repeatEndTime = (frameDuration != -1 && repeatTime + frameDuration - 1 <= repeatRange.end()) ?
                                          repeatTime + frameDuration - 1 : repeatRange.end();
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


KisKeyframeSP KisRepeatFrame::cloneFor(KisKeyframeChannel*) const
{
    return KisKeyframeSP(); // TODO
}

bool KisRepeatFrame::isRepeat(KisKeyframeSP keyframe)
{
    return dynamic_cast<KisRepeatFrame*>(keyframe.data()) != nullptr;
}

KisRepeatFrame::KisRepeatFrame(KisKeyframeChannel *channel, int time, QSharedPointer<KisAnimationCycle> cycle)
        : KisKeyframe(channel, time)
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
