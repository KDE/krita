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

#include "kis_scalar_keyframe_channel.h"
#include "kis_node.h"

struct KisScalarKeyframeChannel::Private
{
public:
    Private(qreal min, qreal max)
        : minValue(min), maxValue(max), firstFreeIndex(0)
    {}

    qreal minValue;
    qreal maxValue;
    QMap<int, qreal> values;
    int firstFreeIndex;
};

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID &id, KisNodeWSP node, qreal minValue, qreal maxValue)
    : KisKeyframeChannel(id, node),
      m_d(new Private(minValue, maxValue))
{
}

KisScalarKeyframeChannel::~KisScalarKeyframeChannel()
{}

bool KisScalarKeyframeChannel::hasScalarValue() const
{
    return true;
}

qreal KisScalarKeyframeChannel::minScalarValue() const
{
    return m_d->minValue;
}

qreal KisScalarKeyframeChannel::maxScalarValue() const
{
    return m_d->maxValue;
}

qreal KisScalarKeyframeChannel::scalarValue(const KisKeyframe *keyframe) const
{
    return m_d->values[keyframe->value()];
}

void KisScalarKeyframeChannel::setScalarValue(KisKeyframe *keyframe, qreal value)
{
    m_d->values[keyframe->value()] = value;
}

KisKeyframe *KisScalarKeyframeChannel::createKeyframe(int time, const KisKeyframe *copySrc)
{
    qreal value = (copySrc != 0) ? scalarValue(copySrc) : 0;
    int index = m_d->firstFreeIndex++;

    m_d->values[index] = value;

    return new KisKeyframe(this, time, index);
}

bool KisScalarKeyframeChannel::canDeleteKeyframe(KisKeyframe *key)
{
    return true;
}

void KisScalarKeyframeChannel::destroyKeyframe(KisKeyframe *key)
{
    m_d->values.remove(key->value());
}

void KisScalarKeyframeChannel::saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement) const
{
    keyframeElement.setAttribute("value", m_d->values[keyframe->value()]);
}

KisKeyframe *KisScalarKeyframeChannel::loadKeyframe(KoXmlNode keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toUInt();
    QVariant value = keyframeNode.toElement().attribute("value");

    KisKeyframe *keyframe = createKeyframe(time, 0);
    setScalarValue(keyframe, value.toReal());

    return keyframe;
}
