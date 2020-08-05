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

    m_frameID = m_paintDevice->framesInterface()->createFrame(false, 0, QPoint(), nullptr);
}

KisRasterKeyframe::KisRasterKeyframe(KisPaintDeviceWSP paintDevice, int premadeFrameID)
    : KisKeyframe()
{
    m_paintDevice = paintDevice;
    m_frameID = premadeFrameID;

    KIS_ASSERT(m_paintDevice);
}

KisRasterKeyframe::~KisRasterKeyframe()
{
    // Note: Because keyframe ownership is shared, it's possible for them to outlive
    // the paint device.
    if (m_paintDevice && m_paintDevice->framesInterface()) {
        m_paintDevice->framesInterface()->deleteFrame(m_frameID, nullptr);
    }
}

int KisRasterKeyframe::frameID() const
{
    return m_frameID;
}

QRect KisRasterKeyframe::contentBounds()
{
    if (!m_paintDevice) {
        return QRect();
    }

    // An empty frame should be the size of a full image.
    if (hasContent()){
        return m_paintDevice->framesInterface()->frameBounds(m_frameID);
    } else {
        return m_paintDevice->defaultBounds()->imageBorderRect();
    }
}

bool KisRasterKeyframe::hasContent()
{
    return !m_paintDevice->framesInterface()->frameBounds(m_frameID).isEmpty();
}

void KisRasterKeyframe::writeFrameToDevice(KisPaintDeviceSP writeTarget)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_paintDevice);

    m_paintDevice->framesInterface()->fetchFrame(m_frameID, writeTarget);
}

KisKeyframeSP KisRasterKeyframe::duplicate(KisKeyframeChannel *newChannel)
{
    if (newChannel) {
        KisRasterKeyframeChannel* rasterChannel = dynamic_cast<KisRasterKeyframeChannel*>(newChannel);
        KIS_ASSERT(rasterChannel);
        KisPaintDeviceWSP targetDevice = rasterChannel->paintDevice();

        if (targetDevice != m_paintDevice) {
            int targetFrameID = targetDevice->framesInterface()->createFrame(false, 0, QPoint(), nullptr);
            targetDevice->framesInterface()->uploadFrame(m_frameID, targetFrameID, m_paintDevice);
            KisKeyframeSP key = toQShared(new KisRasterKeyframe(targetDevice, targetFrameID ));
            key->setColorLabel(colorLabel());
            return key;
        }
    }

    int copyFrameID = m_paintDevice->framesInterface()->createFrame(true, m_frameID, QPoint(), nullptr);
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

    /** @brief Weak pointer to the KisPaintDevice associated with this
     * channel and a single layer of a KisImage. While the channel maintains
     * "virtual" KisRasterKeyframes, the real "physical" frame images are stored
     * within this paint device at the frameID index held in the KisRasterKeyframe. */
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

void KisRasterKeyframeChannel::fetchFrame(int time, KisPaintDeviceSP targetDevice)
{
    KisRasterKeyframeSP key = keyframeAt<KisRasterKeyframe>(time);
    if (!key) {
        key = activeKeyframeAt<KisRasterKeyframe>(time);
    }

    key->writeFrameToDevice(targetDevice);
}

void KisRasterKeyframeChannel::importFrame(int time, KisPaintDeviceSP sourceDevice, KUndo2Command *parentCommand)
{
    addKeyframe(time, parentCommand);
    KisRasterKeyframeSP keyframe = keyframeAt<KisRasterKeyframe>(time);
    m_d->paintDevice->framesInterface()->uploadFrame(keyframe->frameID(), sourceDevice);
}

QRect KisRasterKeyframeChannel::frameExtents(KisKeyframeSP keyframe)
{
    return m_d->paintDevice->framesInterface()->frameBounds(keyframe.dynamicCast<KisRasterKeyframe>()->frameID());
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

QRect KisRasterKeyframeChannel::affectedRect(int time) const
{
    //Note #1: Directionality *not* known
    //Note #2: This function shouldn't fail outright if there is no keyframe at `time`
    QRect affectedRect;

    QList<KisRasterKeyframeSP> relevantFrames;

    relevantFrames.append(keyframeAt<KisRasterKeyframe>(time));
    relevantFrames.append(keyframeAt<KisRasterKeyframe>(nextKeyframeTime(time)));
    relevantFrames.append(keyframeAt<KisRasterKeyframe>(previousKeyframeTime(time)));

    Q_FOREACH (KisRasterKeyframeSP frame, relevantFrames) {
        if (frame) {
            affectedRect |= frame->contentBounds();
        }
    }

    return affectedRect;
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
    KisRasterKeyframeSP rasterKeyframe = keyframe.dynamicCast<KisRasterKeyframe>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(rasterKeyframe);

    int frame = rasterKeyframe->frameID();

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

    KisRasterKeyframeSP keyframe;

    QPoint offset;
    KisDomUtils::loadValue(keyframeNode, "offset", &offset);
    QString frameFilename = keyframeNode.attribute("frame");

    if (m_d->frameFilenames.isEmpty()) {
        // First keyframe loaded: use the existing frame
        KIS_SAFE_ASSERT_RECOVER_NOOP(keyframeCount() == 1);
        int firstKeyframeTime = constKeys().begin().key();
        keyframe = keyframeAt<KisRasterKeyframe>(firstKeyframeTime);

        // Remove from keys. It will get reinserted with new time once we return
        keys().remove(firstKeyframeTime);

        m_d->paintDevice->framesInterface()->setFrameOffset(keyframe->frameID(), offset);
    } else {
        //KUndo2Command tempCommand;
        keyframe = toQShared(new KisRasterKeyframe(m_d->paintDevice));
    }

    setFrameFilename(keyframe->frameID(), frameFilename);

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
