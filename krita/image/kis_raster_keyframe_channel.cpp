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
#include "kis_raster_keyframe_channel.h"
#include "kis_node.h"

#include "kis_paint_device.h"
#include "kis_time_range.h"

struct KisRasterKeyframeChannel::Private
{
  Private(KisPaintDeviceWSP paintDevice)
      : paintDevice(paintDevice)
  {}

  KisPaintDeviceWSP paintDevice;
};

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KoID &id, const KisNodeWSP node, const KisPaintDeviceWSP paintDevice)
    : KisKeyframeChannel(id, node),
      m_d(new Private(paintDevice))
{
    // Raster channels always have at least one frame (representing a static image)
    addKeyframe(0);
}

KisRasterKeyframeChannel::~KisRasterKeyframeChannel()
{}

int KisRasterKeyframeChannel::frameIdAt(int time) const
{
    KisKeyframe *key = activeKeyframeAt(time);
    return key->value();
}

bool KisRasterKeyframeChannel::fetchFrame(KisPaintDeviceSP targetDevice, int time, int offset)
{
    QMap<int, KisKeyframe*>::const_iterator i = activeKeyIterator(time);

    while (offset > 0 && i != constKeys().end()) {
        offset--;
        i++;
    }
    while (offset < 0 && i != constKeys().begin()) {
        offset++;
        i--;
    }

    if (offset != 0 || i == constKeys().end()) return false;

    m_d->paintDevice->fetchFrame(i.value()->value(), targetDevice);

    return true;
}

KisKeyframe *KisRasterKeyframeChannel::createKeyframe(int time, const KisKeyframe *copySrc)
{
    int srcFrame = (copySrc != 0) ? copySrc->value() : 0;

    quint32 frameId = (quint32)m_d->paintDevice->createFrame((copySrc != 0), srcFrame);
    KisKeyframe *keyframe = new KisKeyframe(this, time, frameId);

    return keyframe;
}

bool KisRasterKeyframeChannel::canDeleteKeyframe(KisKeyframe *key)
{
    Q_UNUSED(key);

    // Raster content must have at least one keyframe at all times
    return keys().count() > 1;
}

void KisRasterKeyframeChannel::destroyKeyframe(KisKeyframe *key)
{
    m_d->paintDevice->deleteFrame(key->value());
}

QRect KisRasterKeyframeChannel::affectedRect(KisKeyframe *key)
{
    QMap<int, KisKeyframe *>::iterator it = keys().find(key->time());
    QRect rect;

    // Calculate changed area as the union of the current and previous keyframe.
    // This makes sure there are no artifacts left over from the previous frame
    // where the new one doesn't cover the area.

    if (it == keys().begin()) {
        // Using the *next* keyframe at the start of the timeline avoids artifacts
        // when deleting or moving the first key
        it++;
    } else {
        it--;
    }

    if (it != keys().end()) {
        rect = m_d->paintDevice->frameBounds(it.value()->value());
    }

    rect |= m_d->paintDevice->frameBounds(key->value());

    return rect;
}

void KisRasterKeyframeChannel::requestUpdate(const KisTimeRange &range, const QRect &rect)
{
    KisKeyframeChannel::requestUpdate(range, rect);

    if (range.contains(m_d->paintDevice->defaultBounds()->currentTime())) {
        m_d->paintDevice->setDirty(rect);
    }
}

void KisRasterKeyframeChannel::saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement) const
{
    keyframeElement.setAttribute("frame", keyframe->value());
}

KisKeyframe * KisRasterKeyframeChannel::loadKeyframe(KoXmlNode keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toUInt();
    int frameId = keyframeNode.toElement().attribute("frame").toInt();

    m_d->paintDevice->forceCreateFrame(frameId);

    return new KisKeyframe(this, time, frameId);
}

bool KisRasterKeyframeChannel::hasScalarValue() const
{
    return false;
}

qreal KisRasterKeyframeChannel::minScalarValue() const
{
    return 0;
}

qreal KisRasterKeyframeChannel::maxScalarValue() const
{
    return 0;
}

qreal KisRasterKeyframeChannel::scalarValue(const KisKeyframe *keyframe) const
{
    Q_UNUSED(keyframe);

    return 0;
}

void KisRasterKeyframeChannel::setScalarValue(KisKeyframe *keyframe, qreal value)
{
    Q_UNUSED(keyframe);
    Q_UNUSED(value);
}
