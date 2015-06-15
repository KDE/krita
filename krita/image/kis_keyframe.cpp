/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_keyframe.h"
#include "kis_keyframe_channel.h"

struct KisKeyframe::Private
{
    KisKeyframeChannel *channel;
    quintptr data;
    int time;

    Private(KisKeyframeChannel *channel, int time, quintptr data)
        : channel(channel), time(time), data(data)
    {}
};

KisKeyframe::KisKeyframe(KisKeyframeChannel *channel, int time, void* data)
    : QObject()
    , m_d(new Private(channel, time, (quintptr)data))
{}

KisKeyframe::KisKeyframe(KisKeyframeChannel *channel, int time, quint32 value)
    : QObject()
    , m_d(new Private(channel, time, value))
{}

quint32 KisKeyframe::value() const
{
    return m_d->data;
}

void *KisKeyframe::data() const
{
    return (void*)m_d->data;
}

void KisKeyframe::setValue(quint32 value)
{
    m_d->data = value;
}

int KisKeyframe::time() const
{
    return m_d->time;
}

void KisKeyframe::setTime(int time)
{
    m_d->time = time;
}

KisKeyframeChannel *KisKeyframe::channel() const
{
    return m_d->channel;
}

bool KisKeyframe::affects(int time) const
{
    return wouldAffect(time, this->time());
}

bool KisKeyframe::wouldAffect(int time, int newTime) const
{
    // TODO: think through corner cases..
    KisKeyframe *nextKey = m_d->channel->nextKeyframeAfter(newTime);
    return (newTime <= time && (!nextKey || time < nextKey->time()));
}

#include "kis_keyframe.moc"
