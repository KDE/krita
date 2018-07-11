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

#include "kis_image_config.h"
#include "kis_keyframe.h"
#include "kis_keyframe_channel.h"
#include "kis_types.h"

#include <QPointer>

struct KisKeyframeSPStaticRegistrar {
    KisKeyframeSPStaticRegistrar() {
        qRegisterMetaType<KisKeyframeSP>("KisKeyframeSP");
    }
};
static KisKeyframeSPStaticRegistrar __registrar;


struct KisKeyframe::Private
{
    QPointer<KisKeyframeChannel> channel;
    int time;

    InterpolationMode interpolationMode;
    InterpolationTangentsMode tangentsMode;
    QPointF leftTangent;
    QPointF rightTangent;
    int colorLabel{0};

    Private(KisKeyframeChannel *channel, int time)
        : channel(channel), time(time), interpolationMode(Constant)
    {}
};

KisKeyframe::KisKeyframe(KisKeyframeChannel *channel, int time)
    : m_d(new Private(channel, time))
{
    m_d->colorLabel = KisImageConfig(true).defaultFrameColorLabel();
}

KisKeyframe::KisKeyframe(const KisKeyframe *rhs, KisKeyframeChannel *channel)
    : m_d(new Private(channel, rhs->time()))
{
    m_d->interpolationMode = rhs->m_d->interpolationMode;
    m_d->tangentsMode = rhs->m_d->tangentsMode;
    m_d->leftTangent = rhs->m_d->leftTangent;
    m_d->rightTangent = rhs->m_d->rightTangent;
    m_d->colorLabel = rhs->m_d->colorLabel;
}

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

void KisKeyframe::setTangentsMode(KisKeyframe::InterpolationTangentsMode mode)
{
    m_d->tangentsMode = mode;
}

KisKeyframe::InterpolationTangentsMode KisKeyframe::tangentsMode() const
{
    return m_d->tangentsMode;
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

int KisKeyframe::colorLabel() const
{
    return m_d->colorLabel;
}

void KisKeyframe::setColorLabel(int label)
{
    m_d->colorLabel = label;
}

bool KisKeyframe::hasContent() const {
    return true;
}

KisKeyframeChannel *KisKeyframe::channel() const
{
    return m_d->channel;
}
