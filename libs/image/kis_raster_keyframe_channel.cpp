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
#include "kis_dom_utils.h"

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_paint_device_frames_interface.h"
#include "kis_time_range.h"
#include "kundo2command.h"
#include "kis_onion_skin_compositor.h"

class KisRasterKeyframe : public KisKeyframe
{
public:
    KisRasterKeyframe(KisRasterKeyframeChannel *channel, int time, int frameId)
        : KisKeyframe(channel, time)
        , frameId(frameId)
    {}

    KisRasterKeyframe(const KisRasterKeyframe *rhs, KisKeyframeChannel *channel)
        : KisKeyframe(rhs, channel)
        , frameId(rhs->frameId)
    {}

    int frameId;

    KisKeyframeSP cloneFor(KisKeyframeChannel *channel) const override
    {
        return toQShared(new KisRasterKeyframe(this, channel));
    }

    bool hasContent() const override {
        KisRasterKeyframeChannel *channel = dynamic_cast<KisRasterKeyframeChannel*>(this->channel());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(channel, true);

        return channel->keyframeHasContent(this);
    }
};

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

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KoID &id, const KisPaintDeviceWSP paintDevice, KisDefaultBoundsBaseSP defaultBounds)
    : KisKeyframeChannel(id, defaultBounds),
      m_d(new Private(paintDevice, QString()))
{
}

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KisRasterKeyframeChannel &rhs, KisNode *newParentNode, const KisPaintDeviceWSP newPaintDevice)
    : KisKeyframeChannel(rhs, newParentNode),
      m_d(new Private(newPaintDevice, rhs.m_d->filenameSuffix))
{
    KIS_ASSERT_RECOVER_NOOP(&rhs != this);

    m_d->frameFilenames = rhs.m_d->frameFilenames;
    m_d->onionSkinsEnabled = rhs.m_d->onionSkinsEnabled;
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
    return key->frameId;
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

void KisRasterKeyframeChannel::importFrame(int time, KisPaintDeviceSP sourceDevice, KUndo2Command *parentCommand)
{
    KisKeyframeSP keyframe = addKeyframe(time, parentCommand);

    const int frame = frameId(keyframe);

    m_d->paintDevice->framesInterface()->uploadFrame(frame, sourceDevice);
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

KisKeyframeSP KisRasterKeyframeChannel::createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    KisRasterKeyframe *keyframe;

    if (!copySrc) {
        int frameId = m_d->paintDevice->framesInterface()->createFrame(false, 0, QPoint(), parentCommand);
        keyframe = new KisRasterKeyframe(this, time, frameId);
    } else {
        int srcFrame = frameId(copySrc);
        int frameId = m_d->paintDevice->framesInterface()->createFrame(true, srcFrame, QPoint(), parentCommand);

        KisRasterKeyframe *srcKeyframe = dynamic_cast<KisRasterKeyframe*>(copySrc.data());
        Q_ASSERT(srcKeyframe);
        keyframe = new KisRasterKeyframe(srcKeyframe, this);

        keyframe->setTime(time);
        keyframe->frameId = frameId;
    }

    return toQShared(keyframe);
}

void KisRasterKeyframeChannel::destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand)
{
    m_d->paintDevice->framesInterface()->deleteFrame(frameId(key), parentCommand);
}

void KisRasterKeyframeChannel::uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame)
{
    KisRasterKeyframeChannel *srcRasterChannel = dynamic_cast<KisRasterKeyframeChannel*>(srcChannel);
    KIS_ASSERT_RECOVER_RETURN(srcRasterChannel);

    const int srcId = srcRasterChannel->frameIdAt(srcTime);
    const int dstId = frameId(dstFrame);

    m_d->paintDevice->framesInterface()->
        uploadFrame(srcId,
                    dstId,
                    srcRasterChannel->m_d->paintDevice);
}

QRect KisRasterKeyframeChannel::affectedRect(KisKeyframeSP key)
{
    KeyframesMap::iterator it = keys().find(key->time());
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

    rect |= m_d->paintDevice->framesInterface()->frameBounds(frameId(key));

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

KisKeyframeSP KisRasterKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.attribute("time").toInt();
    workaroundBrokenFrameTimeBug(&time);

    QPoint offset;
    KisDomUtils::loadValue(keyframeNode, "offset", &offset);
    QString frameFilename = keyframeNode.attribute("frame");

    KisKeyframeSP keyframe;

    if (m_d->frameFilenames.isEmpty()) {
        // First keyframe loaded: use the existing frame

        KIS_SAFE_ASSERT_RECOVER_NOOP(keyframeCount() == 1);
        keyframe = constKeys().begin().value();

        // Remove from keys. It will get reinserted with new time once we return
        keys().remove(keyframe->time());

        keyframe->setTime(time);
        m_d->paintDevice->framesInterface()->setFrameOffset(frameId(keyframe), offset);
    } else {
        KUndo2Command tempCommand;
        int frameId = m_d->paintDevice->framesInterface()->createFrame(false, 0, offset, &tempCommand);

        keyframe = toQShared(new KisRasterKeyframe(this, time, frameId));
    }

    setFrameFilename(frameId(keyframe), frameFilename);

    return keyframe;
}

bool KisRasterKeyframeChannel::keyframeHasContent(const KisKeyframe *keyframe) const
{
    return !m_d->paintDevice->framesInterface()->frameBounds(frameId(keyframe)).isEmpty();
}

bool KisRasterKeyframeChannel::hasScalarValue() const
{
    return false;
}

void KisRasterKeyframeChannel::setOnionSkinsEnabled(bool value)
{
    m_d->onionSkinsEnabled = value;
}

bool KisRasterKeyframeChannel::onionSkinsEnabled() const
{
    return m_d->onionSkinsEnabled;
}
