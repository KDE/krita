/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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
#include "kis_dom_utils.h"

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_paint_device_frames_interface.h"
#include "kis_time_span.h"
#include "kundo2command.h"
#include "kis_onion_skin_compositor.h"

KisRasterKeyframe::KisRasterKeyframe(KisPaintDeviceWSP paintDevice)
    : KisKeyframe()
{
    m_paintDevice = paintDevice;
    KIS_ASSERT(m_paintDevice);

    // TODO: make sure command isn't needed.
    m_frameId = m_paintDevice->framesInterface()->createFrame(false, 0, QPoint(), nullptr);
}

KisRasterKeyframe::KisRasterKeyframe(KisPaintDeviceWSP paintDevice, int premadeFrameID)
    : KisKeyframe()
{
    m_paintDevice = paintDevice;
    m_frameId = premadeFrameID;

    KIS_ASSERT(m_paintDevice);
}

KisRasterKeyframe::~KisRasterKeyframe()
{
    // Note: Because keyframe ownership is shared, it's possible for them to outlive
    // the paint device.
    if (m_paintDevice && m_paintDevice->framesInterface()) {
        // TODO: make sure command isn't needed.
        m_paintDevice->framesInterface()->deleteFrame(m_frameId, nullptr);
    }
}

bool KisRasterKeyframe::hasContent()
{
    return !m_paintDevice->framesInterface()->frameBounds(m_frameId).isEmpty();
}

int KisRasterKeyframe::frameID() const
{
    return m_frameId;
}

KisKeyframeSP KisRasterKeyframe::duplicate(KisKeyframeChannel *channel)
{
    if (channel) {
        KisRasterKeyframeChannel* rasterChannel = dynamic_cast<KisRasterKeyframeChannel*>(channel);
        KIS_ASSERT(rasterChannel);
        KisPaintDeviceWSP targetDevice = rasterChannel->paintDevice();

        if (targetDevice != m_paintDevice) {
            int targetFrameID = targetDevice->framesInterface()->createFrame(false, 0, QPoint(), nullptr);
            targetDevice->framesInterface()->uploadFrame(m_frameId, targetFrameID, m_paintDevice);
            KisKeyframeSP key = toQShared(new KisRasterKeyframe(targetDevice, targetFrameID ));
            key->setColorLabel(colorLabel());
            return key;
        }
    }

    int copyFrameID = m_paintDevice->framesInterface()->createFrame(true, m_frameId, QPoint(), nullptr);
    KisKeyframeSP key = toQShared(new KisRasterKeyframe(m_paintDevice, copyFrameID));
    key->setColorLabel(colorLabel());
    return key;
}

// =========================================================================================

struct KisRasterKeyframeChannel::Private
{
    Private(KisPaintDeviceWSP paintDevice, const QString filenameSuffix)
        : paintDevice(paintDevice),
          filenameSuffix(filenameSuffix),
          onionSkinsEnabled(false)
    {}

    KisPaintDeviceWSP paintDevice;
    QMap<int, QString> frameFilenames;
    QString filenameSuffix;
    bool onionSkinsEnabled;
};

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KoID &id, const KisPaintDeviceWSP paintDevice, KisNodeWSP parent)
    : KisKeyframeChannel(id, parent),
      m_d(new Private(paintDevice, QString()))
{
}

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KoID &id, const KisPaintDeviceWSP paintDevice, const KisDefaultBoundsBaseSP bounds)
    : KisKeyframeChannel(id, bounds),
      m_d(new Private(paintDevice, QString()))
{
}

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KisRasterKeyframeChannel &rhs, KisNodeWSP newParent, const KisPaintDeviceWSP newPaintDevice)
    : KisKeyframeChannel(rhs, newParent),
      m_d(new Private(newPaintDevice, rhs.m_d->filenameSuffix))
{
    KIS_ASSERT_RECOVER_NOOP(&rhs != this);

    m_d->frameFilenames = rhs.m_d->frameFilenames;
    m_d->onionSkinsEnabled = rhs.m_d->onionSkinsEnabled;

    Q_FOREACH (int time, rhs.constKeys().keys()) {
        KisRasterKeyframeSP copySource = rhs.keyframeAt<KisRasterKeyframe>(time);
        keys().insert(time, toQShared(new KisRasterKeyframe(newPaintDevice, copySource->frameID())));
    }
}

KisRasterKeyframeChannel::~KisRasterKeyframeChannel()
{
}

int KisRasterKeyframeChannel::frameId(KisKeyframeSP keyframe) const
{
    return frameId(keyframe.data());
}

int KisRasterKeyframeChannel::frameId(const KisKeyframe *keyframe) const
{
    const KisRasterKeyframe *key = dynamic_cast<const KisRasterKeyframe*>(keyframe);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(key, -1);
    return key->frameID();
}

int KisRasterKeyframeChannel::frameIdAt(int time) const
{
    KisKeyframeSP activeKey = activeKeyframeAt(time);
    if (activeKey.isNull()) return -1;
    return frameId(activeKey);
}

void KisRasterKeyframeChannel::fetchFrame(KisKeyframeSP keyframe, KisPaintDeviceSP targetDevice)
{
    m_d->paintDevice->framesInterface()->fetchFrame(frameId(keyframe), targetDevice);
}

void KisRasterKeyframeChannel::fetchFrame(int time, KisPaintDeviceSP targetDevice)
{
    KisRasterKeyframeSP key = keyframeAt<KisRasterKeyframe>(time);
    if (!key) {
        key = activeKeyframeAt<KisRasterKeyframe>(time);
    }

    m_d->paintDevice->framesInterface()->fetchFrame(key->frameID(), targetDevice);
}

void KisRasterKeyframeChannel::importFrame(int time, KisPaintDeviceSP sourceDevice, KUndo2Command *parentCommand)
{
    addKeyframe(time, parentCommand);
    KisRasterKeyframeSP keyframe = keyframeAt<KisRasterKeyframe>(time);
    m_d->paintDevice->framesInterface()->uploadFrame(keyframe->frameID(), sourceDevice);
}

QRect KisRasterKeyframeChannel::frameExtents(KisKeyframeSP keyframe)
{
    return m_d->paintDevice->framesInterface()->frameBounds(frameId(keyframe));
}

QString KisRasterKeyframeChannel::frameFilename(int frameId) const
{
    return m_d->frameFilenames.value(frameId, QString());
}

void KisRasterKeyframeChannel::setFilenameSuffix(const QString &suffix)
{
    m_d->filenameSuffix = suffix;
}

void KisRasterKeyframeChannel::setFrameFilename(int frameId, const QString &filename)
{
    Q_ASSERT(!m_d->frameFilenames.contains(frameId));
    m_d->frameFilenames.insert(frameId, filename);
}

QString KisRasterKeyframeChannel::chooseFrameFilename(int frameId, const QString &layerFilename)
{
    QString filename;

    if (m_d->frameFilenames.isEmpty()) {
        // Use legacy naming convention for first keyframe
        filename = layerFilename + m_d->filenameSuffix;
    } else {
        filename = layerFilename + m_d->filenameSuffix + ".f" + QString::number(frameId);
    }

    setFrameFilename(frameId, filename);

    return filename;
}

QRect KisRasterKeyframeChannel::affectedRect(int time)
{
    KeyframesMap::iterator it = keys().find(time);
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
        rect = m_d->paintDevice->framesInterface()->frameBounds(frameId(it.value()));
    }

    rect |= m_d->paintDevice->framesInterface()->frameBounds(frameIdAt(time));

    if (m_d->onionSkinsEnabled) {
        const QRect dirtyOnionSkinsRect =
            KisOnionSkinCompositor::instance()->calculateFullExtent(m_d->paintDevice);
        rect |= dirtyOnionSkinsRect;
    }

    return rect;
}

QDomElement KisRasterKeyframeChannel::toXML(QDomDocument doc, const QString &layerFilename)
{
    m_d->frameFilenames.clear();

    return KisKeyframeChannel::toXML(doc, layerFilename);
}

void KisRasterKeyframeChannel::loadXML(const QDomElement &channelNode)
{
    m_d->frameFilenames.clear();

    KisKeyframeChannel::loadXML(channelNode);
}

void KisRasterKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    int frame = frameId(keyframe);

    QString filename = frameFilename(frame);
    if (filename.isEmpty()) {
        filename = chooseFrameFilename(frame, layerFilename);
    }
    keyframeElement.setAttribute("frame", filename);

    QPoint offset = m_d->paintDevice->framesInterface()->frameOffset(frame);
    KisDomUtils::saveValue(&keyframeElement, "offset", offset);
}

QPair<int, KisKeyframeSP> KisRasterKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.attribute("time").toInt();
    workaroundBrokenFrameTimeBug(&time);

    KisKeyframeSP keyframe;

    QPoint offset;
    KisDomUtils::loadValue(keyframeNode, "offset", &offset);
    QString frameFilename = keyframeNode.attribute("frame");

    if (m_d->frameFilenames.isEmpty()) {
        // First keyframe loaded: use the existing frame

        KIS_SAFE_ASSERT_RECOVER_NOOP(keyframeCount() == 1);
        int firstKeyframeTime = constKeys().begin().key();
        keyframe = keyframeAt(firstKeyframeTime);

        // Remove from keys. It will get reinserted with new time once we return
        keys().remove(firstKeyframeTime);

        m_d->paintDevice->framesInterface()->setFrameOffset(frameId(keyframe), offset);
    } else {
        //KUndo2Command tempCommand;
        keyframe = toQShared(new KisRasterKeyframe(m_d->paintDevice));
    }

    setFrameFilename(frameId(keyframe), frameFilename);

    return QPair<int, KisKeyframeSP>(time, keyframe);
}

KisKeyframeSP KisRasterKeyframeChannel::createKeyframe()
{
    return toQShared(new KisRasterKeyframe(m_d->paintDevice));
}

void KisRasterKeyframeChannel::setOnionSkinsEnabled(bool value)
{
    m_d->onionSkinsEnabled = value;
}

bool KisRasterKeyframeChannel::onionSkinsEnabled() const
{
    return m_d->onionSkinsEnabled;
}

KisPaintDeviceWSP KisRasterKeyframeChannel::paintDevice()
{
    return m_d->paintDevice;
}
