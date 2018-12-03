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
#include "kis_keyframe_commands.h"

struct KisRasterKeyframeChannel::Private
{
    Private(KisPaintDeviceWSP paintDevice, const QString filenameSuffix)
            : paintDevice(paintDevice),
            filenameSuffix(filenameSuffix),
            onionSkinsEnabled(false)
    {}

    KisPaintDeviceWSP paintDevice;
    QMap<QString, int> framesByFilename;
    QMap<int, QString> frameFilenames;
    QMap<int, QVector<KisKeyframeSP>> frameInstances;
    QString filenameSuffix;
    bool onionSkinsEnabled;
};

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

    QRect affectedRect() const override
    {
        KisRasterKeyframeChannel *ch = dynamic_cast<KisRasterKeyframeChannel *>(channel());
        KisRasterKeyframeChannel::Private *ch_d = ch->m_d.data();

        // Calculate changed area as the union of the current and previous keyframe.
        // This makes sure there are no artifacts left over from the previous frame
        // where the new one doesn't cover the area.
        KisKeyframeSP neighbor = ch->previousKeyframe(*this);

        // Using the *next* keyframe at the start of the timeline avoids artifacts
        // when deleting or moving the first key
        if (neighbor.isNull()) neighbor = ch->nextKeyframe(*this);

        QRect rect = ch_d->paintDevice->framesInterface()->frameBounds(frameId);

        if (!neighbor.isNull()) {
            // Note: querying through frameIdAt makes sure cycle repeats resolve to their original frames
            const int neighborFrameId = ch->frameIdAt(neighbor->time());
            rect |= ch_d->paintDevice->framesInterface()->frameBounds(neighborFrameId);
        }

        if (ch_d->onionSkinsEnabled) {
            const QRect dirtyOnionSkinsRect =
                    KisOnionSkinCompositor::instance()->calculateFullExtent(ch_d->paintDevice);
            rect |= dirtyOnionSkinsRect;
        }

        return rect;
    }

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

KisKeyframeSP KisRasterKeyframeChannel::linkKeyframe(const KisKeyframeBaseSP source, int newTime, KUndo2Command *parentCommand)
{
    KisKeyframeSP sourceKeyframe = source.dynamicCast<KisKeyframe>();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(sourceKeyframe, KisKeyframeSP());

    const int frame = frameId(sourceKeyframe);
    KisKeyframeSP newKeyframe = toQShared(new KisRasterKeyframe(this, newTime, frame));
    m_d->frameInstances[frame].append(newKeyframe);

    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, newTime, newKeyframe, parentCommand);
    cmd->redo();

    return newKeyframe;
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
    KisKeyframeSP activeKey = visibleKeyframeAt(time);
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
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->frameFilenames.contains(frameId));
    m_d->frameFilenames.insert(frameId, filename);
    m_d->framesByFilename.insert(filename, frameId);
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

    const bool copy = !copySrc.isNull();
    const int srcFrameId = copy ? frameId(copySrc) : 0;
    const int frameId = m_d->paintDevice->framesInterface()->createFrame(copy, srcFrameId, QPoint(), parentCommand);

    if (!copy) {
        keyframe = new KisRasterKeyframe(this, time, frameId);
    } else {
        const KisRasterKeyframe *srcKeyframe = dynamic_cast<KisRasterKeyframe*>(copySrc.data());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(srcKeyframe, KisKeyframeSP());

        keyframe = new KisRasterKeyframe(srcKeyframe, this);
        keyframe->setTime(time);
        keyframe->frameId = frameId;
    }

    KisKeyframeSP keyframeSP = toQShared(keyframe);

    m_d->frameInstances[keyframe->frameId].append(keyframeSP);

    return keyframeSP;
}

void KisRasterKeyframeChannel::destroyKeyframe(KisKeyframeSP keyframe, KUndo2Command *parentCommand)
{
    int id = frameId(keyframe);

    QVector<KisKeyframeSP> &instances = m_d->frameInstances[id];
    instances.removeAll(keyframe);

    if (instances.isEmpty()) {
        m_d->frameInstances.remove(id);
        m_d->paintDevice->framesInterface()->deleteFrame(id, parentCommand);
    }
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
        const int id = frameId(keyframe);
        setFrameFilename(id, frameFilename);

        // Remove from keys. It will get reinserted with new time once we return
        keys().remove(keyframe->time());

        keyframe->setTime(time);
        m_d->paintDevice->framesInterface()->setFrameOffset(id, offset);
    } else {
        int frameId = m_d->framesByFilename.value(frameFilename, -1);

        if (frameId == -1) {
            KUndo2Command tempCommand;
            frameId = m_d->paintDevice->framesInterface()->createFrame(false, 0, offset, &tempCommand);
            setFrameFilename(frameId, frameFilename);
        }

        keyframe = toQShared(new KisRasterKeyframe(this, time, frameId));
        m_d->frameInstances[frameId].append(keyframe);
    }

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

KisFrameSet KisRasterKeyframeChannel::affectedFrames(int time) const
{
    const int frameId = frameIdAt(time);

    if (frameId < 0) {
        // Not a raster frame (e.g. repeat of a cycle)
        return KisKeyframeChannel::affectedFrames(time);
    }

    KisFrameSet frames;
    Q_FOREACH(KisKeyframeSP keyframe, m_d->frameInstances[frameId]) {
        frames |= KisKeyframeChannel::affectedFrames(keyframe->time());
    }
    return frames;
}

KisFrameSet KisRasterKeyframeChannel::identicalFrames(int time, KisTimeSpan range) const
{
    const int frameId = frameIdAt(time);

    KisFrameSet frames;
    Q_FOREACH(KisKeyframeSP keyframe, m_d->frameInstances[frameId]) {
        frames |= KisKeyframeChannel::identicalFrames(keyframe->time(), range);
    }
    return frames;
}

void KisRasterKeyframeChannel::setOnionSkinsEnabled(bool value)
{
    m_d->onionSkinsEnabled = value;
}

bool KisRasterKeyframeChannel::onionSkinsEnabled() const
{
    return m_d->onionSkinsEnabled;
}
