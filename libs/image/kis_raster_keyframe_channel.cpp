/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_layer_utils.h"

KisRasterKeyframe::KisRasterKeyframe(KisPaintDeviceWSP paintDevice)
    : KisKeyframe()
{
    m_paintDevice = paintDevice;
    KIS_ASSERT(m_paintDevice);

    m_frameID = m_paintDevice->framesInterface()->createFrame(false, 0, QPoint(), nullptr);
}

KisRasterKeyframe::KisRasterKeyframe(KisPaintDeviceWSP paintDevice, const int &premadeFrameID, const int &colorLabelId)
    : KisKeyframe()
{
    m_paintDevice = paintDevice;
    m_frameID = premadeFrameID;
    setColorLabel(colorLabelId);

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

    return m_paintDevice->framesInterface()->frameBounds(m_frameID);
}

bool KisRasterKeyframe::hasContent()
{
    return !m_paintDevice->framesInterface()->frameBounds(m_frameID).isEmpty();
}

void KisRasterKeyframe::writeFrameToDevice(KisPaintDeviceSP writeTarget)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_paintDevice);

    m_paintDevice->framesInterface()->writeFrameToDevice(m_frameID, writeTarget);
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


// ===========================================================================================================
// =======================================KisRasterKeyframeChannel============================================
// ===========================================================================================================


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

    QMultiHash<int, int> frameIDTimesMap;

    QMap<int, QString> frameFilenames;
    QString filenameSuffix;
    bool onionSkinsEnabled;
};

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KoID &id, const KisPaintDeviceWSP paintDevice, const KisDefaultBoundsBaseSP bounds)
    : KisKeyframeChannel(id, bounds),
      m_d(new Private(paintDevice, QString()))
{
}

KisRasterKeyframeChannel::KisRasterKeyframeChannel(const KisRasterKeyframeChannel &rhs, const KisPaintDeviceWSP newPaintDevice)
    : KisKeyframeChannel(rhs),
      m_d(new Private(newPaintDevice, rhs.m_d->filenameSuffix))
{
    KIS_ASSERT_RECOVER_NOOP(&rhs != this);

    m_d->frameFilenames = rhs.m_d->frameFilenames;
    m_d->onionSkinsEnabled = rhs.m_d->onionSkinsEnabled;

    // Copy keyframes with attention to clones..
    foreach (const int& frame, rhs.constKeys().keys()) {
        KisRasterKeyframeSP copySource = rhs.keyframeAt<KisRasterKeyframe>(frame);
        if (m_d->frameIDTimesMap.contains(copySource->frameID())){
            continue;
        }

        KisRasterKeyframeSP transferredKey = toQShared(new KisRasterKeyframe(newPaintDevice, copySource->frameID(), copySource->colorLabel()));
        foreach (const int& time, rhs.m_d->frameIDTimesMap.values(transferredKey->frameID())) {
            keys().insert(time, transferredKey);
            m_d->frameIDTimesMap.insert(transferredKey->frameID(), time);
        }
    }
}

KisRasterKeyframeChannel::~KisRasterKeyframeChannel()
{
}

void KisRasterKeyframeChannel::writeToDevice(int time, KisPaintDeviceSP targetDevice)
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

void KisRasterKeyframeChannel::insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentUndoCmd)
{
    KisKeyframeChannel::insertKeyframe(time, keyframe, parentUndoCmd);

    KisRasterKeyframeSP rasterKey = keyframe.dynamicCast<KisRasterKeyframe>();
    if (rasterKey) {
        m_d->frameIDTimesMap.insert(rasterKey->frameID(), time);
    }
}

void KisRasterKeyframeChannel::removeKeyframe(int time, KUndo2Command *parentUndoCmd)
{
    KisRasterKeyframeSP rasterKey = keyframeAt<KisRasterKeyframe>(time);
    if (rasterKey) {
        m_d->frameIDTimesMap.remove(rasterKey->frameID(), time);
    }

    KisKeyframeChannel::removeKeyframe(time, parentUndoCmd);

    if (time == 0) { // There should always be a raster frame on frame 0.
        addKeyframe(time, parentUndoCmd);
    }
}

void KisRasterKeyframeChannel::cloneKeyframe(int source, int destination, KUndo2Command *parentUndoCmd)
{
    if (!keyframeAt(source)) return;

    insertKeyframe(destination, keyframeAt<KisRasterKeyframe>(source), parentUndoCmd);
}

bool KisRasterKeyframeChannel::areClones(int timeA, int timeB)
{
    /* Edgecase
     * If both times are empty, we shouldn't really consider the two "clones".. */
    if (keyframeAt(timeA) == nullptr && keyframeAt(timeB) == nullptr) {
        return false;
    }

    return (keyframeAt(timeA) == keyframeAt(timeB));
}

QSet<int> KisRasterKeyframeChannel::clonesOf(int time)
{
    KisRasterKeyframeSP rasterKey = keyframeAt<KisRasterKeyframe>(time);

    if (!rasterKey) {
        return QSet<int>();
    }

    QList<int> values = m_d->frameIDTimesMap.values(rasterKey->frameID());
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    QSet<int> clones = QSet<int>(values.cbegin(), values.cend());
#else
    QSet<int> clones = QSet<int>::fromList(values);
#endif
    clones.remove(time); // Clones only! Remove input time from the list.
    return clones;
}

QSet<int> KisRasterKeyframeChannel::timesForFrameID(int frameID) const
{
    QSet<int> clones;
    if (m_d->frameIDTimesMap.contains(frameID)) {
        QList<int> values = m_d->frameIDTimesMap.values(frameID);
        #if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        clones = QSet<int>(values.cbegin(), values.cend());
        #else
        clones = QSet<int>::fromList(values);
        #endif
    }
    return clones;
}

QSet<int> KisRasterKeyframeChannel::clonesOf(const KisNode *node, int time)
{
    QSet<int> clones;

    QMap<QString, KisKeyframeChannel*> chans = node->keyframeChannels();
    foreach (KisKeyframeChannel* channel, chans.values()){
        KisRasterKeyframeChannel* rasterChan = dynamic_cast<KisRasterKeyframeChannel*>(channel);
        if (!rasterChan) {
            continue;
        }

        QSet<int> chanClones = rasterChan->clonesOf(rasterChan->activeKeyframeTime(time));
        clones += chanClones;
    }

    return clones;
}

void KisRasterKeyframeChannel::makeUnique(int time, KUndo2Command* parentUndoCmd)
{
    KisRasterKeyframeSP rasterKey = keyframeAt<KisRasterKeyframe>(time);

    if (rasterKey && clonesOf(time).count() > 0) {
        insertKeyframe(time, rasterKey->duplicate(), parentUndoCmd);
    }
}

QRect KisRasterKeyframeChannel::affectedRect(int time) const
{
    QRect affectedRect;

    QList<KisRasterKeyframeSP> relevantFrames;
    relevantFrames.append(keyframeAt<KisRasterKeyframe>(time));
    relevantFrames.append(keyframeAt<KisRasterKeyframe>(previousKeyframeTime(time)));

    Q_FOREACH (KisRasterKeyframeSP frame, relevantFrames) {
        if (frame) {
            affectedRect |= frame->contentBounds();
        }
    }

    return affectedRect;
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
        removeKeyframe(firstKeyframeTime);
        m_d->paintDevice->framesInterface()->setFrameOffset(keyframe->frameID(), offset);
    } else {

        // If the filename already exists, it's **probably** a clone we can reinstance.
        if (m_d->frameFilenames.values().contains(frameFilename)) {

            const int frameId = m_d->frameFilenames.key(frameFilename);
            const int cloneOf = m_d->frameIDTimesMap.values(frameId).first();
            const KisRasterKeyframeSP instance = keyframeAt<KisRasterKeyframe>(cloneOf);
            return QPair<int, KisKeyframeSP>(time, instance);
        } else {

            keyframe = toQShared(new KisRasterKeyframe(m_d->paintDevice));
            m_d->paintDevice->framesInterface()->setFrameOffset(keyframe->frameID(), offset);
        }
    }

    setFrameFilename(keyframe->frameID(), frameFilename);

    return QPair<int, KisKeyframeSP>(time, keyframe);
}

KisKeyframeSP KisRasterKeyframeChannel::createKeyframe()
{
    return toQShared(new KisRasterKeyframe(m_d->paintDevice));
}
