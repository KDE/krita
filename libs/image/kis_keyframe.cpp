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

#include <QPointer>


struct KisKeyframe::Private
{
    QPointer<KisKeyframeChannel> channel;
    int time;

    InterpolationMode interpolationMode;
    QPointF leftTangent;
    QPointF rightTangent;

    Private(KisKeyframeChannel *channel, int time)
        : channel(channel), time(time), interpolationMode(Constant)
    {}
};

KisKeyframe::KisKeyframe(KisKeyframeChannel *channel, int time)
    : m_d(new Private(channel, time))
{}


KisKeyframe::~KisKeyframe()
{}

int KisKeyframe::time() const
{
    return m_d->time;
}

void KisKeyframe::setTime(int time)
{
    m_d->time = time;
}

void KisKeyframe::setInterpolationMode(KisKeyframe::InterpolationMode mode)
{
    m_d->interpolationMode = mode;
}

KisKeyframe::InterpolationMode KisKeyframe::interpolationMode() const
{
    return m_d->interpolationMode;
}

void KisKeyframe::setInterpolationTangents(QPointF leftTangent, QPointF rightTangent)
{
    m_d->leftTangent = leftTangent;
    m_d->rightTangent = rightTangent;
}

QPointF KisKeyframe::leftTangent() const
{
    return m_d->leftTangent;
}

QPointF KisKeyframe::rightTangent() const
{
    return m_d->rightTangent;
}

KisKeyframeChannel *KisKeyframe::channel() const
{
    return m_d->channel;
}
