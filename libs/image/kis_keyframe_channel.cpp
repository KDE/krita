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

#include "kis_keyframe_channel.h"
#include "KoID.h"
#include "kis_global.h"
#include "kis_node.h"
#include "kis_time_range.h"
#include "kundo2command.h"
#include "kis_keyframe_commands.h"

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
    Private(const Private &rhs, KisNodeWSP newParentNode) {
        node = newParentNode;
        id = rhs.id;
        defaultBounds = rhs.defaultBounds;
        haveBrokenFrameTimeBug = rhs.haveBrokenFrameTimeBug;
    }

    KeyframesMap keys;
    KisNodeWSP node;
    KoID id;
    KisDefaultBoundsBaseSP defaultBounds;
    bool haveBrokenFrameTimeBug = false;
};

KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisDefaultBoundsBaseSP defaultBounds)
    : m_d(new Private)
{
    m_d->id = id;
    m_d->node = 0;
    m_d->defaultBounds = defaultBounds;
}

KisKeyframeChannel::KisKeyframeChannel(const KisKeyframeChannel &rhs, KisNode *newParentNode)
    : m_d(new Private(*rhs.m_d, newParentNode))
{
    KIS_ASSERT_RECOVER_NOOP(&rhs != this);

    Q_FOREACH(KisKeyframeSP keyframe, rhs.m_d->keys) {
        m_d->keys.insert(keyframe->time(), keyframe->cloneFor(this));
    }
}

KisKeyframeChannel::~KisKeyframeChannel()
{}

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
}

KisNodeWSP KisKeyframeChannel::node() const
{
    return m_d->node;
}

int KisKeyframeChannel::keyframeCount() const
{
    return m_d->keys.count();
}

KisKeyframeChannel::KeyframesMap& KisKeyframeChannel::keys()
{
    return m_d->keys;
}

const KisKeyframeChannel::KeyframesMap& KisKeyframeChannel::constKeys() const
{
    return m_d->keys;
}

#define LAZY_INITIALIZE_PARENT_COMMAND(cmd)       \
    QScopedPointer<KUndo2Command> __tempCommand;  \
    if (!parentCommand) {                         \
        __tempCommand.reset(new KUndo2Command()); \
        cmd = __tempCommand.data();               \
    }

KisKeyframeSP KisKeyframeChannel::addKeyframe(int time, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);
    return insertKeyframe(time, KisKeyframeSP(), parentCommand);
}

KisKeyframeSP KisKeyframeChannel::copyKeyframe(const KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);
    return insertKeyframe(newTime, keyframe, parentCommand);
}

KisKeyframeSP KisKeyframeChannel::linkKeyframe(const KisKeyframeSP, int, KUndo2Command*) {
    return KisKeyframeSP();
}

KisKeyframeSP KisKeyframeChannel::insertKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    KisKeyframeSP keyframe = keyframeAt(time);
    if (keyframe) {
        deleteKeyframeImpl(keyframe, parentCommand, false);
    }

    Q_ASSERT(parentCommand);
    keyframe = createKeyframe(time, copySrc, parentCommand);

    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, keyframe->time(), keyframe, parentCommand);
    cmd->redo();

    return keyframe;
}

bool KisKeyframeChannel::deleteKeyframe(KisKeyframeSP keyframe, KUndo2Command *parentCommand)
{
    return deleteKeyframeImpl(keyframe, parentCommand, true);
}

bool KisKeyframeChannel::moveKeyframe(KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    if (newTime == keyframe->time()) return false;

    KisKeyframeSP other = keyframeAt(newTime);
    if (other) {
        deleteKeyframeImpl(other, parentCommand, false);
    }

    const int srcTime = keyframe->time();

    KUndo2Command *cmd = new KisMoveFrameCommand(this, keyframe, srcTime, newTime, parentCommand);
    cmd->redo();

    if (srcTime == 0) {
        addKeyframe(srcTime, parentCommand);
    }

    return true;
}

bool KisKeyframeChannel::swapFrames(int lhsTime, int rhsTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    if (lhsTime == rhsTime) return false;

    KisKeyframeSP lhsFrame = keyframeAt(lhsTime);
    KisKeyframeSP rhsFrame = keyframeAt(rhsTime);

    if (!lhsFrame && !rhsFrame) return false;

    if (lhsFrame && !rhsFrame) {
        moveKeyframe(lhsFrame, rhsTime, parentCommand);
    } else if (!lhsFrame && rhsFrame) {
        moveKeyframe(rhsFrame, lhsTime, parentCommand);
    } else {
        KUndo2Command *cmd = new KisSwapFramesCommand(this, lhsFrame, rhsFrame, parentCommand);
        cmd->redo();
    }

    return true;
}

bool KisKeyframeChannel::deleteKeyframeImpl(KisKeyframeSP keyframe, KUndo2Command *parentCommand, bool recreate)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    Q_ASSERT(parentCommand);

    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, keyframe->time(), KisKeyframeSP(), parentCommand);
    cmd->redo();
    destroyKeyframe(keyframe, parentCommand);

    if (recreate && keyframe->time() == 0) {
        addKeyframe(0, parentCommand);
    }

    return true;
}

void KisKeyframeChannel::moveKeyframeImpl(KisKeyframeSP keyframe, int newTime)
{
    KIS_ASSERT_RECOVER_RETURN(keyframe);
    KIS_ASSERT_RECOVER_RETURN(!keyframeAt(newTime));

    KisTimeRange rangeSrc = affectedFrames(keyframe->time());
    QRect rectSrc = affectedRect(keyframe);

    emit sigKeyframeAboutToBeMoved(keyframe, newTime);

    m_d->keys.remove(keyframe->time());
    int oldTime = keyframe->time();
    keyframe->setTime(newTime);
    m_d->keys.insert(newTime, keyframe);

    emit sigKeyframeMoved(keyframe, oldTime);

    KisTimeRange rangeDst = affectedFrames(keyframe->time());
    QRect rectDst = affectedRect(keyframe);

    requestUpdate(rangeSrc, rectSrc);
    requestUpdate(rangeDst, rectDst);
}

void KisKeyframeChannel::swapKeyframesImpl(KisKeyframeSP lhsKeyframe, KisKeyframeSP rhsKeyframe)
{
    KIS_ASSERT_RECOVER_RETURN(lhsKeyframe);
    KIS_ASSERT_RECOVER_RETURN(rhsKeyframe);

    KisTimeRange rangeLhs = affectedFrames(lhsKeyframe->time());
    KisTimeRange rangeRhs = affectedFrames(rhsKeyframe->time());

    const QRect rectLhsSrc = affectedRect(lhsKeyframe);
    const QRect rectRhsSrc = affectedRect(rhsKeyframe);

    const int lhsTime = lhsKeyframe->time();
    const int rhsTime = rhsKeyframe->time();

    emit sigKeyframeAboutToBeMoved(lhsKeyframe, rhsTime);
    emit sigKeyframeAboutToBeMoved(rhsKeyframe, lhsTime);

    m_d->keys.remove(lhsTime);
    m_d->keys.remove(rhsTime);

    rhsKeyframe->setTime(lhsTime);
    lhsKeyframe->setTime(rhsTime);

    m_d->keys.insert(lhsTime, rhsKeyframe);
    m_d->keys.insert(rhsTime, lhsKeyframe);

    emit sigKeyframeMoved(lhsKeyframe, lhsTime);
    emit sigKeyframeMoved(rhsKeyframe, rhsTime);

    const QRect rectLhsDst = affectedRect(lhsKeyframe);
    const QRect rectRhsDst = affectedRect(rhsKeyframe);

    requestUpdate(rangeLhs, rectLhsSrc | rectRhsDst);
    requestUpdate(rangeRhs, rectRhsSrc | rectLhsDst);
}

KisKeyframeSP KisKeyframeChannel::replaceKeyframeAt(int time, KisKeyframeSP newKeyframe)
{
    Q_ASSERT(newKeyframe.isNull() || time == newKeyframe->time());

    KisKeyframeSP existingKeyframe = keyframeAt(time);
    if (!existingKeyframe.isNull()) {
        removeKeyframeLogical(existingKeyframe);
    }

    if (!newKeyframe.isNull()) {
        insertKeyframeLogical(newKeyframe);
    }

    return existingKeyframe;
}

void KisKeyframeChannel::insertKeyframeLogical(KisKeyframeSP keyframe)
{
    const int time = keyframe->time();

    emit sigKeyframeAboutToBeAdded(keyframe);
    m_d->keys.insert(time, keyframe);
    emit sigKeyframeAdded(keyframe);

    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(time);
    requestUpdate(range, rect);
}

void KisKeyframeChannel::removeKeyframeLogical(KisKeyframeSP keyframe)
{
    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(keyframe->time());

    emit sigKeyframeAboutToBeRemoved(keyframe);
    m_d->keys.remove(keyframe->time());
    emit sigKeyframeRemoved(keyframe);

    requestUpdate(range, rect);
}

KisKeyframeSP KisKeyframeChannel::keyframeAt(int time) const
{
    KeyframesMap::const_iterator i = m_d->keys.constFind(time);
    if (i != m_d->keys.constEnd()) {
        return i.value();
    }

    return KisKeyframeSP();
}

KisKeyframeSP KisKeyframeChannel::activeKeyframeAt(int time) const
{
    KeyframesMap::const_iterator i = activeKeyIterator(time);
    if (i != m_d->keys.constEnd()) {
        return i.value();
    }

    return KisKeyframeSP();
}

KisKeyframeSP KisKeyframeChannel::currentlyActiveKeyframe() const
{
    return activeKeyframeAt(currentTime());
}

KisKeyframeSP KisKeyframeChannel::firstKeyframe() const
{
    if (m_d->keys.isEmpty()) return KisKeyframeSP();
    return m_d->keys.first();
}

KisKeyframeSP KisKeyframeChannel::nextKeyframe(KisKeyframeSP keyframe) const
{
    KeyframesMap::const_iterator i = m_d->keys.constFind(keyframe->time());
    if (i == m_d->keys.constEnd()) return KisKeyframeSP(0);

    i++;

    if (i == m_d->keys.constEnd()) return KisKeyframeSP(0);
    return i.value();
}

KisKeyframeSP KisKeyframeChannel::previousKeyframe(KisKeyframeSP keyframe) const
{
    KeyframesMap::const_iterator i = m_d->keys.constFind(keyframe->time());
    if (i == m_d->keys.constBegin() || i == m_d->keys.constEnd()) return KisKeyframeSP(0);
    i--;

    return i.value();
}

KisKeyframeSP KisKeyframeChannel::lastKeyframe() const
{
    if (m_d->keys.isEmpty()) return KisKeyframeSP(0);

    return (m_d->keys.end()-1).value();
}

void KisKeyframeChannel::activeKeyframeRange(int time, int *first, int *last) const
{
    *first = *last = -1;

    const KisKeyframeSP currentKeyframe = activeKeyframeAt(time);
    if (currentKeyframe.isNull()) return;

    *first = currentKeyframe->time();

    const KisKeyframeSP next = nextKeyframe(currentKeyframe);
    if (!next.isNull()) {
        *last = next->time() - 1;
    }
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

QSet<int> KisKeyframeChannel::allKeyframeIds() const
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

KisTimeRange KisKeyframeChannel::affectedFrames(int time) const
{
    if (m_d->keys.isEmpty()) return KisTimeRange::infinite(0);

    KeyframesMap::const_iterator active = activeKeyIterator(time);
    KeyframesMap::const_iterator next;

    int from;

    if (active == m_d->keys.constEnd()) {
        // No active keyframe, ie. time is before the first keyframe
        from = 0;
        next = m_d->keys.constBegin();
    } else {
        from = active.key();
        next = active + 1;
    }

    if (next == m_d->keys.constEnd()) {
        return KisTimeRange::infinite(from);
    } else {
        return KisTimeRange::fromTime(from, next.key() - 1);
    }
}

KisTimeRange KisKeyframeChannel::identicalFrames(int time) const
{
    KeyframesMap::const_iterator active = activeKeyIterator(time);

    if (active != m_d->keys.constEnd() && (active+1) != m_d->keys.constEnd()) {
        if (active->data()->interpolationMode() != KisKeyframe::Constant) {
            return KisTimeRange::fromTime(time, time);
        }
    }

    return affectedFrames(time);
}

int KisKeyframeChannel::keyframeRowIndexOf(KisKeyframeSP keyframe) const
{
    KeyframesMap::const_iterator it = m_d->keys.constBegin();
    KeyframesMap::const_iterator end = m_d->keys.constEnd();

    int row = 0;

    for (; it != end; ++it) {
        if (it.value().data() == keyframe) {
            return row;
        }

        row++;
    }

    return -1;
}

KisKeyframeSP KisKeyframeChannel::keyframeAtRow(int row) const
{
    KeyframesMap::const_iterator it = m_d->keys.constBegin();
    KeyframesMap::const_iterator end = m_d->keys.constEnd();

    for (; it != end; ++it) {
        if (row <= 0) {
            return it.value();
        }

        row--;
    }

    return KisKeyframeSP();
}

int KisKeyframeChannel::keyframeInsertionRow(int time) const
{
    KeyframesMap::const_iterator it = m_d->keys.constBegin();
    KeyframesMap::const_iterator end = m_d->keys.constEnd();

    int row = 0;

    for (; it != end; ++it) {
        if (it.value()->time() > time) {
            break;
        }
        row++;
    }

    return row;
}

QDomElement KisKeyframeChannel::toXML(QDomDocument doc, const QString &layerFilename)
{
    QDomElement channelElement = doc.createElement("channel");

    channelElement.setAttribute("name", id());

    Q_FOREACH (KisKeyframeSP keyframe, m_d->keys.values()) {
        QDomElement keyframeElement = doc.createElement("keyframe");
        keyframeElement.setAttribute("time", keyframe->time());
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

        KisKeyframeSP keyframe = loadKeyframe(keyframeNode);
        KIS_SAFE_ASSERT_RECOVER(keyframe) { continue; }

        if (keyframeNode.hasAttribute("color-label")) {
            keyframe->setColorLabel(keyframeNode.attribute("color-label").toUInt());
        }

        m_d->keys.insert(keyframe->time(), keyframe);
    }
}

bool KisKeyframeChannel::swapExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, int dstTime, KUndo2Command *parentCommand)
{
    if (srcChannel->id() != id()) {
        warnKrita << "Cannot copy frames from different ids:" << ppVar(srcChannel->id()) << ppVar(id());
        return KisKeyframeSP();
    }

    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    KisKeyframeSP srcFrame = srcChannel->keyframeAt(srcTime);
    KisKeyframeSP dstFrame = keyframeAt(dstTime);

    if (!dstFrame && srcFrame) {
        copyExternalKeyframe(srcChannel, srcTime, dstTime, parentCommand);
        srcChannel->deleteKeyframe(srcFrame, parentCommand);
    } else if (dstFrame && !srcFrame) {
        srcChannel->copyExternalKeyframe(this, dstTime, srcTime, parentCommand);
        deleteKeyframe(dstFrame, parentCommand);
    } else if (dstFrame && srcFrame) {
        const int fakeFrameTime = -1;

        KisKeyframeSP newKeyframe = createKeyframe(fakeFrameTime, KisKeyframeSP(), parentCommand);
        uploadExternalKeyframe(srcChannel, srcTime, newKeyframe);

        srcChannel->copyExternalKeyframe(this, dstTime, srcTime, parentCommand);

        // do not recreate frame!
        deleteKeyframeImpl(dstFrame, parentCommand, false);

        newKeyframe->setTime(dstTime);

        KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, newKeyframe->time(), newKeyframe, parentCommand);
        cmd->redo();
    }

    return true;
}


KisKeyframeSP KisKeyframeChannel::copyExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, int dstTime, KUndo2Command *parentCommand)
{
    if (srcChannel->id() != id()) {
        warnKrita << "Cannot copy frames from different ids:" << ppVar(srcChannel->id()) << ppVar(id());
        return KisKeyframeSP();
    }

    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    KisKeyframeSP dstFrame = keyframeAt(dstTime);
    if (dstFrame) {
        deleteKeyframeImpl(dstFrame, parentCommand, false);
    }

    KisKeyframeSP newKeyframe = createKeyframe(dstTime, KisKeyframeSP(), parentCommand);
    uploadExternalKeyframe(srcChannel, srcTime, newKeyframe);

    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, newKeyframe->time(), newKeyframe, parentCommand);
    cmd->redo();

    return newKeyframe;
}

KisKeyframeChannel::KeyframesMap::const_iterator
KisKeyframeChannel::activeKeyIterator(int time) const
{
    KeyframesMap::const_iterator i = const_cast<const KeyframesMap*>(&m_d->keys)->upperBound(time);

    if (i == m_d->keys.constBegin()) return m_d->keys.constEnd();
    return --i;
}

void KisKeyframeChannel::requestUpdate(const KisTimeRange &range, const QRect &rect)
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

int KisKeyframeChannel::currentTime() const
{
    return m_d->defaultBounds->currentTime();
}

qreal KisKeyframeChannel::minScalarValue() const
{
    return 0;
}

qreal KisKeyframeChannel::maxScalarValue() const
{
    return 0;
}

qreal KisKeyframeChannel::scalarValue(const KisKeyframeSP keyframe) const
{
    Q_UNUSED(keyframe);

    return 0;
}

void KisKeyframeChannel::setScalarValue(KisKeyframeSP keyframe, qreal value, KUndo2Command *parentCommand)
{
    Q_UNUSED(keyframe);
    Q_UNUSED(value);
    Q_UNUSED(parentCommand);
}
