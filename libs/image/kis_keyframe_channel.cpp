/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_keyframe_channel.h"
#include "KoID.h"
#include "kis_global.h"
#include "kis_node.h"
#include "kis_time_span.h"
#include "kundo2command.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_keyframe_commands.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_mask.h"
#include "kis_image.h"

#include <QMap>


const KoID KisKeyframeChannel::Raster = KoID("content", ki18n("Content"));
const KoID KisKeyframeChannel::Opacity = KoID("opacity", ki18n("Opacity"));
const KoID KisKeyframeChannel::PositionX = KoID("transform_pos_x", ki18n("Position (X)"));
const KoID KisKeyframeChannel::PositionY = KoID("transform_pos_y", ki18n("Position (Y)"));
const KoID KisKeyframeChannel::ScaleX = KoID("transform_scale_x", ki18n("Scale (X)"));
const KoID KisKeyframeChannel::ScaleY = KoID("transform_scale_y", ki18n("Scale (Y)"));
const KoID KisKeyframeChannel::ShearX = KoID("transform_shear_x", ki18n("Shear (X)"));
const KoID KisKeyframeChannel::ShearY = KoID("transform_shear_y", ki18n("Shear (Y)"));
const KoID KisKeyframeChannel::RotationX = KoID("transform_rotation_x", ki18n("Rotation (X)"));
const KoID KisKeyframeChannel::RotationY = KoID("transform_rotation_y", ki18n("Rotation (Y)"));
const KoID KisKeyframeChannel::RotationZ = KoID("transform_rotation_z", ki18n("Rotation (Z)"));


struct KisKeyframeChannel::Private
{
    Private(const KoID &temp_id, KisDefaultBoundsBaseSP bounds) {
        bounds = bounds;
        id = temp_id;
    }

    Private(const Private &rhs) {
        id = rhs.id;
        haveBrokenFrameTimeBug = rhs.haveBrokenFrameTimeBug;
    }

    KoID id;
    QMap<int, KisKeyframeSP> keys; /**< Maps unique times to individual keyframes. */
    KisDefaultBoundsBaseSP bounds; /**< Stores pixel dimensions as well as current time. */

    KisNodeWSP parentNode;
    bool haveBrokenFrameTimeBug = false;
};


KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisDefaultBoundsBaseSP bounds)
    : m_d(new Private(id, bounds))
{
    // Added keyframes should fire channel updated signal..
    connect(this, &KisKeyframeChannel::sigAddedKeyframe, [](const KisKeyframeChannel *channel, int time) {
        channel->sigChannelUpdated(
                    channel->affectedFrames(time),
                    channel->affectedRect(time)
                    );
    });

    // Removing keyframes should fire channel updated signal..
    connect(this, &KisKeyframeChannel::sigRemovingKeyframe, [](const KisKeyframeChannel *channel, int time) {
        channel->sigChannelUpdated(
                   channel->affectedFrames(time),
                   channel->affectedRect(time)
                   );
    });
}

KisKeyframeChannel::KisKeyframeChannel(const KisKeyframeChannel &rhs)
    : KisKeyframeChannel(rhs.m_d->id, new KisDefaultBounds(nullptr))
{
    m_d.reset(new Private(*rhs.m_d));
}

KisKeyframeChannel::~KisKeyframeChannel()
{
}

void KisKeyframeChannel::addKeyframe(int time, KUndo2Command *parentUndoCmd)
{
    KisKeyframeSP keyframe = createKeyframe();
    insertKeyframe(time, keyframe, parentUndoCmd);
}

void KisKeyframeChannel::insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentUndoCmd)
{
    KIS_ASSERT(time >= 0);
    KIS_ASSERT(keyframe);

    if (m_d->keys.contains(time)) {
        // Properly remove overwritten frames.
        removeKeyframe(time, parentUndoCmd);
    }

    if (parentUndoCmd) {
        KUndo2Command* cmd = new KisInsertKeyframeCommand(this, time, keyframe, parentUndoCmd);
        Q_UNUSED(cmd);
    }

    m_d->keys.insert(time, keyframe);
    emit sigAddedKeyframe(this, time);
}

void KisKeyframeChannel::removeKeyframe(int time, KUndo2Command *parentUndoCmd)
{
    if (parentUndoCmd) {
        KUndo2Command* cmd = new KisRemoveKeyframeCommand(this, time, parentUndoCmd);
        Q_UNUSED(cmd);
    }

    emit sigRemovingKeyframe(this, time);
    m_d->keys.remove(time);
}

void KisKeyframeChannel::moveKeyframe(KisKeyframeChannel *sourceChannel, int sourceTime, KisKeyframeChannel *targetChannel, int targetTime, KUndo2Command *parentUndoCmd)
{
    KIS_ASSERT(sourceChannel && targetChannel);

    KisKeyframeSP sourceKeyframe = sourceChannel->keyframeAt(sourceTime);
    sourceChannel->removeKeyframe(sourceTime, parentUndoCmd);

    KisKeyframeSP targetKeyframe = sourceKeyframe;
    if (sourceChannel != targetChannel) {
        // When "moving" Keyframes between channels, a new copy is made for that channel.
        targetKeyframe = sourceKeyframe->duplicate(targetChannel);
    }

    targetChannel->insertKeyframe(targetTime, targetKeyframe, parentUndoCmd);
}

void KisKeyframeChannel::copyKeyframe(const KisKeyframeChannel *sourceChannel, int sourceTime, KisKeyframeChannel *targetChannel, int targetTime, KUndo2Command* parentUndoCmd)
{
    KIS_ASSERT(sourceChannel && targetChannel);

    KisKeyframeSP sourceKeyframe = sourceChannel->keyframeAt(sourceTime);
    KisKeyframeSP copiedKeyframe = sourceKeyframe->duplicate(targetChannel);

    targetChannel->insertKeyframe(targetTime, copiedKeyframe, parentUndoCmd);
}

void KisKeyframeChannel::swapKeyframes(KisKeyframeChannel *channelA, int timeA, KisKeyframeChannel *channelB, int timeB, KUndo2Command *parentUndoCmd)
{
    KIS_ASSERT(channelA && channelB);

    // Store B.
    KisKeyframeSP keyframeB = channelB->keyframeAt(timeB);

    // Move A -> B
    moveKeyframe(channelA, timeA, channelB, timeB, parentUndoCmd);

    // Insert B -> A
    if (channelA != channelB) {
        keyframeB = keyframeB->duplicate(channelA);
    }
    channelA->insertKeyframe(timeA, keyframeB, parentUndoCmd);
}

KisKeyframeSP KisKeyframeChannel::keyframeAt(int time) const
{
    QMap<int, KisKeyframeSP>::const_iterator iter = m_d->keys.constFind(time);
    if (iter != m_d->keys.constEnd()) {
        return iter.value();
    } else {
        return KisKeyframeSP();
    }
}

int KisKeyframeChannel::keyframeCount() const
{
    return m_d->keys.count();
}

int KisKeyframeChannel::activeKeyframeTime(int time) const
{
    QMap<int, KisKeyframeSP>::const_iterator iter = const_cast<const QMap<int, KisKeyframeSP>*>(&m_d->keys)->upperBound(time);

    // If the next keyframe is the first keyframe, that means there's no active frame.
    if (iter == m_d->keys.constBegin()) {
        return -1;
    }

    iter--;

    if (iter == m_d->keys.constEnd()) {
        return -1;
    }

    return iter.key();
}

int KisKeyframeChannel::firstKeyframeTime() const
{
    if (m_d->keys.isEmpty()) {
        return -1;
    } else {
        return m_d->keys.firstKey();
    }
}

int KisKeyframeChannel::previousKeyframeTime(const int time) const
{
    if (!keyframeAt(time)) {
        return activeKeyframeTime(time);
    }

    QMap<int, KisKeyframeSP>::const_iterator iter = m_d->keys.constFind(time);

    if (iter == m_d->keys.constBegin() || iter == m_d->keys.constEnd()) {
        return -1;
    }

    iter--;
    return iter.key();
}

int KisKeyframeChannel::nextKeyframeTime(const int time) const
{
    QMap<int, KisKeyframeSP>::const_iterator iter = const_cast<const QMap<int, KisKeyframeSP>*>(&m_d->keys)->upperBound(time);

    if (iter == m_d->keys.constEnd()) {
        return -1;
    }

    return iter.key();
}

int KisKeyframeChannel::lastKeyframeTime() const
{
    if (m_d->keys.isEmpty()) {
        return -1;
    }

    return m_d->keys.lastKey();
}

QSet<int> KisKeyframeChannel::allKeyframeTimes() const
{
    QSet<int> frames;

    TimeKeyframeMap::const_iterator it = m_d->keys.constBegin();
    TimeKeyframeMap::const_iterator end = m_d->keys.constEnd();

    while (it != end) {
        frames.insert(it.key());
        ++it;
    }

    return frames;
}

QString KisKeyframeChannel::id() const
{
    return m_d->id.id();
}

QString KisKeyframeChannel::name() const
{
    return m_d->id.name();
}

void KisKeyframeChannel::setNode(KisNodeWSP node)
{
    if (m_d->parentNode.isValid()) { // Disconnect old..
        disconnect(this, &KisKeyframeChannel::sigChannelUpdated, m_d->parentNode, &KisNode::handleKeyframeChannelUpdate);
    }

    m_d->parentNode = node;
    m_d->bounds = KisDefaultBoundsNodeWrapperSP( new KisDefaultBoundsNodeWrapper( node ));

    if (m_d->parentNode) { // Connect new..
        connect(this, &KisKeyframeChannel::sigChannelUpdated, m_d->parentNode, &KisNode::handleKeyframeChannelUpdate);
    }
}

KisNodeWSP KisKeyframeChannel::node() const
{
    return m_d->parentNode;
}

void KisKeyframeChannel::setDefaultBounds(KisDefaultBoundsBaseSP bounds) {
    m_d->bounds = bounds;
}

int KisKeyframeChannel::channelHash() const
{
    TimeKeyframeMap::const_iterator it = m_d->keys.constBegin();
    TimeKeyframeMap::const_iterator end = m_d->keys.constEnd();

    int hash = 0;

    while (it != end) {
        hash += it.key();
        ++it;
    }

    return hash;
}

KisTimeSpan KisKeyframeChannel::affectedFrames(int time) const
{
    if (m_d->keys.isEmpty()) return KisTimeSpan::infinite(0);

    const int activeKeyTime = activeKeyframeTime(time);
    const int nextKeyTime = nextKeyframeTime(time);

    // Check for keyframe behind..
    if (!keyframeAt(activeKeyTime)) {
        return KisTimeSpan::fromTimeToTime(0, nextKeyTime - 1);
    }

    // Check for keyframe ahead..
    if (!keyframeAt(nextKeyTime)) {
        return KisTimeSpan::infinite(activeKeyTime);
    }

    return KisTimeSpan::fromTimeToTime(activeKeyTime, nextKeyTime - 1);
}

KisTimeSpan KisKeyframeChannel::identicalFrames(int time) const
{
    return affectedFrames(time);
}

QDomElement KisKeyframeChannel::toXML(QDomDocument doc, const QString &layerFilename)
{
    QDomElement channelElement = doc.createElement("channel");

    channelElement.setAttribute("name", id());

    Q_FOREACH (int time, m_d->keys.keys()) {
        QDomElement keyframeElement = doc.createElement("keyframe");
        KisKeyframeSP keyframe = keyframeAt(time);

        keyframeElement.setAttribute("time", time);
        keyframeElement.setAttribute("color-label", keyframe->colorLabel());

        saveKeyframe(keyframe, keyframeElement, layerFilename);

        channelElement.appendChild(keyframeElement);
    }

    return channelElement;
}

void KisKeyframeChannel::loadXML(const QDomElement &channelNode)
{
    for (QDomElement keyframeNode = channelNode.firstChildElement(); !keyframeNode.isNull(); keyframeNode = keyframeNode.nextSiblingElement()) {
        if (keyframeNode.nodeName().toUpper() != "KEYFRAME") continue;

        QPair<int, KisKeyframeSP> timeKeyPair = loadKeyframe(keyframeNode);
        KIS_SAFE_ASSERT_RECOVER(timeKeyPair.second) { continue; }

        if (keyframeNode.hasAttribute("color-label")) {
            timeKeyPair.second->setColorLabel(keyframeNode.attribute("color-label").toUInt());
        }

        insertKeyframe(timeKeyPair.first, timeKeyPair.second);
    }
}

KisKeyframeChannel::TimeKeyframeMap& KisKeyframeChannel::keys()
{
    return m_d->keys;
}

const KisKeyframeChannel::TimeKeyframeMap& KisKeyframeChannel::constKeys() const
{
    return m_d->keys;
}

int KisKeyframeChannel::currentTime() const
{
    return m_d->bounds->currentTime();
}

void KisKeyframeChannel::workaroundBrokenFrameTimeBug(int *time)
{
    if (*time < 0) {
        qWarning() << "WARNING: Loading a file with negative animation frames!";
        qWarning() << "         The file has been saved with a buggy version of Krita.";
        qWarning() << "         All the frames with negative ids will be dropped!";
        qWarning() << "         " << ppVar(this->id()) << ppVar(*time);

        m_d->haveBrokenFrameTimeBug = true;
        *time = 0;
    }

    if (m_d->haveBrokenFrameTimeBug) {
        while (keyframeAt(*time)) {
            (*time)++;
        }
    }
}
