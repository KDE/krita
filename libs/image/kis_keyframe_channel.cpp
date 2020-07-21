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

#include "kis_keyframe_channel.h"
#include "KoID.h"
#include "kis_global.h"
#include "kis_node.h"
#include "kis_time_span.h"
#include "kundo2command.h"
#include "kis_keyframe_commands.h"
#include "kis_scalar_keyframe_channel.h"

#include <QMap>


const KoID KisKeyframeChannel::Content = KoID("content", ki18n("Content"));
const KoID KisKeyframeChannel::Opacity = KoID("opacity", ki18n("Opacity"));
const KoID KisKeyframeChannel::TransformArguments = KoID("transform_arguments", ki18n("Transform"));
const KoID KisKeyframeChannel::TransformPositionX = KoID("transform_pos_x", ki18n("Position (X)"));
const KoID KisKeyframeChannel::TransformPositionY = KoID("transform_pos_y", ki18n("Position (Y)"));
const KoID KisKeyframeChannel::TransformScaleX = KoID("transform_scale_x", ki18n("Scale (X)"));
const KoID KisKeyframeChannel::TransformScaleY = KoID("transform_scale_y", ki18n("Scale (Y)"));
const KoID KisKeyframeChannel::TransformShearX = KoID("transform_shear_x", ki18n("Shear (X)"));
const KoID KisKeyframeChannel::TransformShearY = KoID("transform_shear_y", ki18n("Shear (Y)"));
const KoID KisKeyframeChannel::TransformRotationX = KoID("transform_rotation_x", ki18n("Rotation (X)"));
const KoID KisKeyframeChannel::TransformRotationY = KoID("transform_rotation_y", ki18n("Rotation (Y)"));
const KoID KisKeyframeChannel::TransformRotationZ = KoID("transform_rotation_z", ki18n("Rotation (Z)"));


struct KisKeyframeChannel::Private
{
    Private() {}
    Private(const Private &rhs) {
        id = rhs.id;
        haveBrokenFrameTimeBug = rhs.haveBrokenFrameTimeBug;
    }

    QMap<int, KisKeyframeSP> keys;
    KisNodeWSP node;
    KoID id;
    KisDefaultBoundsBaseSP defaultBounds;
    bool haveBrokenFrameTimeBug = false;
};


KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisNodeWSP parent)
    : m_d(new Private())
{
    m_d->id = id;
    m_d->node = parent;
    m_d->defaultBounds = KisDefaultBoundsNodeWrapperSP( new KisDefaultBoundsNodeWrapper( parent ));
}

KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisDefaultBoundsBaseSP bounds)
    : m_d(new Private())
{
    m_d->id = id;
    m_d->node = nullptr;
    m_d->defaultBounds = bounds;
}

KisKeyframeChannel::KisKeyframeChannel(const KisKeyframeChannel &rhs, KisNodeWSP newParent)
    : m_d(new Private(*rhs.m_d))
{
    KIS_ASSERT_RECOVER_NOOP(&rhs != this);

    m_d->node = newParent;
    m_d->defaultBounds = KisDefaultBoundsNodeWrapperSP( new KisDefaultBoundsNodeWrapper( newParent ));
}

KisKeyframeChannel::~KisKeyframeChannel()
{
}

void KisKeyframeChannel::addKeyframe(int time, KUndo2Command *parentCmd)
{
    KisKeyframeSP keyframe = createKeyframe();
    insertKeyframe(time, keyframe, parentCmd);
}

void KisKeyframeChannel::insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentCmd)
{
    KIS_ASSERT(keyframe);
    if (parentCmd) {
        KUndo2Command* cmd = new KisInsertKeyframeCommand(this, time, keyframe, parentCmd);
    }

    m_d->keys.insert(time, keyframe);
}

void KisKeyframeChannel::removeKeyframe(int time, KUndo2Command *parentCmd)
{
    if (parentCmd) {
        KUndo2Command* cmd = new KisRemoveKeyframeCommand(this, time, parentCmd);
    }

    m_d->keys.remove(time);

    if (time == 0) { // There should always be a frame on frame 0.
        addKeyframe(time);
    }
}

void KisKeyframeChannel::moveKeyframe(KisKeyframeChannel *sourceChannel, int sourceTime, KisKeyframeChannel *targetChannel, int targetTime, KUndo2Command *parentCmd)
{
    KIS_ASSERT(sourceChannel && targetChannel);

    KisKeyframeSP sourceKeyframe = sourceChannel->keyframeAt(sourceTime);
    sourceChannel->removeKeyframe(sourceTime, parentCmd);

    KisKeyframeSP targetKeyframe = sourceKeyframe;
    if (sourceChannel != targetChannel) {
        // When "moving" Keyframes between channels, a new copy is made for that channel.
        targetKeyframe = sourceKeyframe->duplicate(targetChannel);
    }

    targetChannel->insertKeyframe(targetTime, targetKeyframe, parentCmd);
}

void KisKeyframeChannel::copyKeyframe(KisKeyframeChannel *sourceChannel, int sourceTime, KisKeyframeChannel *targetChannel, int targetTime, KUndo2Command* parentCmd)
{
    KIS_ASSERT(sourceChannel && targetChannel);

    KisKeyframeSP sourceKeyframe = sourceChannel->keyframeAt(sourceTime);
    KisKeyframeSP copiedKeyframe = sourceKeyframe->duplicate(targetChannel);

    targetChannel->insertKeyframe(targetTime, copiedKeyframe, parentCmd);
}

void KisKeyframeChannel::swapKeyframes(KisKeyframeChannel *channelA, int timeA, KisKeyframeChannel *channelB, int timeB, KUndo2Command *parentCmd)
{
    KIS_ASSERT(channelA && channelB);

    // Store B.
    KisKeyframeSP keyframeB = channelB->keyframeAt(timeB);

    // Move A -> B
    moveKeyframe(channelA, timeA, channelB, timeB, parentCmd);

    // Insert B -> A
    if (channelA != channelB) {
        keyframeB = keyframeB->duplicate(channelA);
    }
    channelA->insertKeyframe(timeA, keyframeB, parentCmd);
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
    m_d->node = node;
    m_d->defaultBounds = KisDefaultBoundsNodeWrapperSP( new KisDefaultBoundsNodeWrapper( node ));
}

KisNodeWSP KisKeyframeChannel::node() const
{
    return m_d->node;
}

int KisKeyframeChannel::keyframeCount() const
{
    return m_d->keys.count();
}

QSet<int> KisKeyframeChannel::allKeyframeTimes() const
{
    QSet<int> frames;

    KeyframesMap::const_iterator it = m_d->keys.constBegin();
    KeyframesMap::const_iterator end = m_d->keys.constEnd();

    while (it != end) {
        frames.insert(it.key());
        ++it;
    }

    return frames;
}

int KisKeyframeChannel::framesHash() const
{
    KeyframesMap::const_iterator it = m_d->keys.constBegin();
    KeyframesMap::const_iterator end = m_d->keys.constEnd();

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

    KeyframesMap::const_iterator active = activeKeyIterator(time);
    KeyframesMap::const_iterator next;

    // ie. time is before the first keyframe
    const bool noActiveKeyframe = (active == m_d->keys.constEnd());

    int from;

    if (noActiveKeyframe) {
        from = 0;
        next = m_d->keys.constBegin();
    } else {
        from = active.key();
        next = active + 1;
    }

    if (next == m_d->keys.constEnd()) {
        return KisTimeSpan::infinite(from);
    } else {
        KisScalarKeyframeSP activeScalar = active.value().dynamicCast<KisScalarKeyframe>();
        const KisScalarKeyframe::InterpolationMode activeMode = (noActiveKeyframe || !activeScalar) ? KisScalarKeyframe::Constant :
                                                                activeScalar->interpolationMode();

        if (activeMode == KisScalarKeyframe::Constant) {
            return KisTimeSpan::fromTime(from, next.key() - 1);
        } else {
            return KisTimeSpan::fromTime(from, from);
        }
    }
}

KisTimeSpan KisKeyframeChannel::identicalFrames(int time) const
{
    KeyframesMap::const_iterator active = activeKeyIterator(time);

    if (active != m_d->keys.constEnd() && (active+1) != m_d->keys.constEnd()) {
        KisScalarKeyframeSP activeScalar = active.value().dynamicCast<KisScalarKeyframe>();
        if (activeScalar && activeScalar->interpolationMode() != KisScalarKeyframe::Constant) {
            return KisTimeSpan::fromTime(time, time);
        }
    }

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

        m_d->keys.insert(timeKeyPair.first, timeKeyPair.second);
    }
}

KisKeyframeChannel::KeyframesMap& KisKeyframeChannel::keys()
{
    return m_d->keys;
}

const KisKeyframeChannel::KeyframesMap& KisKeyframeChannel::constKeys() const
{
    return m_d->keys;
}

int KisKeyframeChannel::currentTime() const
{
    return m_d->defaultBounds->currentTime();
}

void KisKeyframeChannel::requestUpdate(const KisTimeSpan &range, const QRect &rect)
{
    if (m_d->node) {
        m_d->node->invalidateFrames(range, rect);

        int currentTime = m_d->defaultBounds->currentTime();
        if (range.contains(currentTime)) {
            m_d->node->setDirty(rect);
        }
    }
}

void KisKeyframeChannel::workaroundBrokenFrameTimeBug(int *time)
{
    /**
     * Between Krita 4.1 and 4.4 Krita had a bug which resulted in creating frames
     * with negative time stamp. The bug has been fixed, but there might be some files
     * still in the wild.
     *
     * TODO: remove this workaround in Krita 5.0, when no such file are left :)
     */

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

KisKeyframeChannel::KeyframesMap::const_iterator KisKeyframeChannel::activeKeyIterator(int time) const
{
    KeyframesMap::const_iterator i = const_cast<const KeyframesMap*>(&m_d->keys)->upperBound(time);

    if (i == m_d->keys.constBegin()) return m_d->keys.constEnd();
    return --i;
}
