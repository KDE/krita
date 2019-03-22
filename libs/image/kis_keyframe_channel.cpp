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
#include "KisCollectionUtils.h"
#include "kis_time_range.h"
#include "kundo2command.h"
#include "kis_keyframe_commands.h"
#include "kis_animation_cycle.h"

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

/**
 * Finds the last item the in the map with a key less than the given one.
 * Returns map.constEnd() if no such key exists.
 */
template<class T>
typename QMap<int, T>::const_iterator findActive(const QMap<int, T> &map, int maximumKey)
{
    typename QMap<int, T>::const_iterator i = map.upperBound(maximumKey);
    if (i == map.constBegin()) return map.constEnd();
    return i - 1;
}

/**
 * Finds the last item the in the map before the "active" item (see findActive) for the given key.
 * Returns map.constEnd() if no such key exists.
 */
template<class T>
typename QMap<int, T>::const_iterator findPrevious(const QMap<int, T> &map, int currentKey)
{
    typename QMap<int, T>::const_iterator active = lastBeforeOrAt(map, currentKey);
    if (active == map.constEnd()) return map.constEnd();

    if (currentKey > active.key()) return active;

    if (active == map.constBegin()) return map.constEnd();
    return active - 1;
}

/**
 * Finds the first item the in the map with a key greater than the given one.
 * Returns map.constEnd() if no such key exists.
 */
template<class T>
typename QMap<int, T>::const_iterator findNext(const QMap<int, T> &map, int currentKey)
{
    return map.upperBound(currentKey);
}

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
    QMap<int, QSharedPointer<KisAnimationCycle>> cycles;
    QMap<int, QSharedPointer<KisRepeatFrame>> repeats;
    KisNodeWSP node;
    KoID id;
    KisDefaultBoundsBaseSP defaultBounds;
    bool haveBrokenFrameTimeBug = false;

    void add(KisKeyframeBaseSP item) {
        auto repeat = item.dynamicCast<KisRepeatFrame>();
        if (repeat) {
            repeat->cycle()->addRepeat(repeat);
            repeats.insert(repeat->time(), repeat);
        } else {
            auto keyframe = item.dynamicCast<KisKeyframe>();
            KIS_ASSERT_RECOVER_RETURN(keyframe);
            keys.insert(item->time(), keyframe);
        }
    }

    void remove(KisKeyframeBaseSP item) {
        auto repeat = item.dynamicCast<KisRepeatFrame>();
        if (repeat) {
            repeat->cycle()->removeRepeat(repeat);
            repeats.remove(repeat->time());
        } else {
            keys.remove(item->time());
        }
    }

    void moveKeyframe(KisKeyframeBaseSP keyframe, int newTime) {
        const QSharedPointer<KisAnimationCycle> cycle = cycles.value(keyframe->time());

        remove(keyframe);
        keyframe->setTime(newTime);
        add(keyframe);

        if (cycle) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(newTime < cycle->lastSourceKeyframe()->time()); // TODO: make sure this is the case

            cycles.remove(cycle->time());
            cycle->setTime(newTime);
            cycles.insert(newTime, cycle);
        }
    }

    void addCycle(QSharedPointer<KisAnimationCycle> cycle) {
        cycles.insert(cycle->time(), cycle);

        Q_FOREACH(QWeakPointer<KisRepeatFrame> repeatWP, cycle->repeats()) {
            KisKeyframeBaseSP repeat = repeatWP.toStrongRef();
            if (repeat) add(repeat);
        }
    }

    void removeCycle(QSharedPointer<KisAnimationCycle> cycle) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(cycle->repeats().isEmpty());
        cycles.remove(cycle->time());
    }
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
        const KisKeyframeSP clone = keyframe->cloneFor(this);
            m_d->add(clone);
    }

    Q_FOREACH(const QSharedPointer<KisAnimationCycle> rhsCycle, rhs.m_d->cycles) {
        KisKeyframeSP firstFrame = keyframeAt(rhsCycle->firstSourceKeyframe()->time());
        KisKeyframeSP lastFrame = keyframeAt(rhsCycle->lastSourceKeyframe()->time());

        QSharedPointer<KisAnimationCycle> cycle = toQShared(new KisAnimationCycle(this, firstFrame, lastFrame));
        m_d->addCycle(cycle);

        Q_FOREACH(auto rhsRepeatWP, rhsCycle->repeats()) {
            const QSharedPointer<KisRepeatFrame> rhsRepeat = rhsRepeatWP.toStrongRef();
            if (rhsRepeat) {
                m_d->add(toQShared(new KisRepeatFrame(this, rhsRepeat->time(), cycle)));
            }
        }

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

KisKeyframeBaseSP KisKeyframeChannel::copyItem(const KisKeyframeBaseSP item, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);
    return insertKeyframe(newTime, item, parentCommand);
}

KisKeyframeSP KisKeyframeChannel::copyAsKeyframe(const KisKeyframeBaseSP item, int originalTime, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    KisKeyframeSP keyframe = item->getOriginalKeyframeFor(originalTime);
    return insertKeyframe(newTime, keyframe, parentCommand);
}

KisKeyframeSP KisKeyframeChannel::linkKeyframe(const KisKeyframeBaseSP, int, KUndo2Command*) {
    return KisKeyframeSP();
}

KisKeyframeSP KisKeyframeChannel::insertKeyframe(int time, const KisKeyframeBaseSP copySrc, KUndo2Command *parentCommand)
{
    KisKeyframeBaseSP oldItem = itemAt(time);
    if (oldItem) {
        deleteKeyframeImpl(oldItem, parentCommand, false);
    }

    Q_ASSERT(parentCommand);
    KisKeyframeSP sourceKeyframe = copySrc ? copySrc->getOriginalKeyframeFor(copySrc->time()) : KisKeyframeSP();
    KisKeyframeSP keyframe = createKeyframe(time, sourceKeyframe, parentCommand);

    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, keyframe->time(), keyframe, parentCommand);
    cmd->redo();

    return keyframe;
}

bool KisKeyframeChannel::deleteKeyframe(KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand)
{
    return deleteKeyframeImpl(keyframe, parentCommand, true);
}

bool KisKeyframeChannel::moveKeyframe(KisKeyframeBaseSP keyframe, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    if (newTime == keyframe->time()) return false;

    KisKeyframeBaseSP other = itemAt(newTime);
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

    KisKeyframeBaseSP lhsFrame = itemAt(lhsTime);
    KisKeyframeBaseSP rhsFrame = itemAt(rhsTime);

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

bool KisKeyframeChannel::deleteKeyframeImpl(KisKeyframeBaseSP item, KUndo2Command *parentCommand, bool recreate)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    Q_ASSERT(parentCommand);

    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, item->time(), KisKeyframeSP(), parentCommand);
    cmd->redo();

    KisKeyframeSP keyframe = item.dynamicCast<KisKeyframe>();
    if (keyframe) {
        destroyKeyframe(keyframe, parentCommand);

        if (recreate && keyframe->time() == 0) {
            addKeyframe(0, parentCommand);
        }
    }

    return true;
}

void KisKeyframeChannel::moveKeyframeImpl(KisKeyframeBaseSP keyframe, int newTime)
{
    KIS_ASSERT_RECOVER_RETURN(keyframe);
    KIS_ASSERT_RECOVER_RETURN(!itemAt(newTime));

    KisFrameSet rangeSrc = affectedFrames(keyframe->time());
    QRect rectSrc = keyframe->affectedRect();

    emit sigKeyframeAboutToBeMoved(keyframe, newTime);

    int oldTime = keyframe->time();
    m_d->moveKeyframe(keyframe, newTime);
    keyframe->setTime(newTime);

    emit sigKeyframeMoved(keyframe, oldTime);

    KisFrameSet rangeDst = affectedFrames(keyframe->time());
    QRect rectDst = keyframe->affectedRect();

    requestUpdate(rangeSrc, rectSrc);
    requestUpdate(rangeDst, rectDst);
}

void KisKeyframeChannel::swapKeyframesImpl(KisKeyframeBaseSP lhsKeyframe, KisKeyframeBaseSP rhsKeyframe)
{
    KIS_ASSERT_RECOVER_RETURN(lhsKeyframe);
    KIS_ASSERT_RECOVER_RETURN(rhsKeyframe);

    KisFrameSet rangeLhs = affectedFrames(lhsKeyframe->time());
    KisFrameSet rangeRhs = affectedFrames(rhsKeyframe->time());

    const QRect rectLhsSrc = lhsKeyframe->affectedRect();
    const QRect rectRhsSrc = rhsKeyframe->affectedRect();

    const int lhsTime = lhsKeyframe->time();
    const int rhsTime = rhsKeyframe->time();

    emit sigKeyframeAboutToBeMoved(lhsKeyframe, rhsTime);
    emit sigKeyframeAboutToBeMoved(rhsKeyframe, lhsTime);

    m_d->remove(lhsKeyframe);
    m_d->remove(rhsKeyframe);

    rhsKeyframe->setTime(lhsTime);
    lhsKeyframe->setTime(rhsTime);

    m_d->add(lhsKeyframe);
    m_d->add(rhsKeyframe);

    emit sigKeyframeMoved(lhsKeyframe, lhsTime);
    emit sigKeyframeMoved(rhsKeyframe, rhsTime);

    const QRect rectLhsDst = lhsKeyframe->affectedRect();
    const QRect rectRhsDst = rhsKeyframe->affectedRect();

    requestUpdate(rangeLhs, rectLhsSrc | rectRhsDst);
    requestUpdate(rangeRhs, rectRhsSrc | rectLhsDst);
}

KisKeyframeBaseSP KisKeyframeChannel::replaceKeyframeAt(int time, KisKeyframeBaseSP newKeyframe)
{
    Q_ASSERT(newKeyframe.isNull() || time == newKeyframe->time());

    KisKeyframeBaseSP existingKeyframe = itemAt(time);
    if (!existingKeyframe.isNull()) {
        removeKeyframeLogical(existingKeyframe);
    }

    if (!newKeyframe.isNull()) {
        insertKeyframeLogical(newKeyframe);
    }

    return existingKeyframe;
}

void KisKeyframeChannel::insertKeyframeLogical(KisKeyframeBaseSP keyframe)
{
    const int time = keyframe->time();

    emit sigKeyframeAboutToBeAdded(keyframe);
    m_d->add(keyframe);
    emit sigKeyframeAdded(keyframe);

    QRect rect = keyframe->affectedRect();
    KisFrameSet range = affectedFrames(time);
    requestUpdate(range, rect);
}

void KisKeyframeChannel::removeKeyframeLogical(KisKeyframeBaseSP keyframe)
{
    QRect rect = keyframe->affectedRect();
    KisFrameSet range = affectedFrames(keyframe->time());

    emit sigKeyframeAboutToBeRemoved(keyframe);
    m_d->remove(keyframe);
    emit sigKeyframeRemoved(keyframe);

    requestUpdate(range, rect);
}

KisKeyframeSP KisKeyframeChannel::keyframeAt(int time) const
{
    return m_d->keys.value(time);
}

KisKeyframeSP KisKeyframeChannel::activeKeyframeAt(int time) const
{
    KeyframesMap::const_iterator i = KisCollectionUtils::lastBeforeOrAt(m_d->keys, time);
    if (i != m_d->keys.constEnd()) {
        return i.value();
    }

    return KisKeyframeSP();
}

KisKeyframeSP KisKeyframeChannel::visibleKeyframeAt(int time) const
{
    const QSharedPointer<KisRepeatFrame> repeat = activeRepeatAt(time);
    return repeat ? repeat->getOriginalKeyframeFor(time) : activeKeyframeAt(time);
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
    return nextKeyframe(keyframe->time());
}

KisKeyframeSP KisKeyframeChannel::nextKeyframe(int time) const
{
    KeyframesMap::const_iterator i = KisCollectionUtils::firstAfter(m_d->keys, time);
    if (i == m_d->keys.constEnd()) return KisKeyframeSP(0);
    return i.value();
}

KisKeyframeSP KisKeyframeChannel::previousKeyframe(KisKeyframeSP keyframe) const
{
    return previousKeyframe(keyframe->time());
}

KisKeyframeSP KisKeyframeChannel::previousKeyframe(int time) const
{
    KeyframesMap::const_iterator i = KisCollectionUtils::lastBefore(m_d->keys, time);
    if (i == m_d->keys.constEnd()) return KisKeyframeSP(0);
    return i.value();
}

KisKeyframeSP KisKeyframeChannel::lastKeyframe() const
{
    if (m_d->keys.isEmpty()) return KisKeyframeSP(0);

    return (m_d->keys.end()-1).value();
}

KisKeyframeBaseSP KisKeyframeChannel::itemAt(int time) const
{
    const KisKeyframeSP keyframe = keyframeAt(time);
    if (keyframe) return keyframe;

    const QSharedPointer<KisRepeatFrame> repeat = activeRepeatAt(time);
    if (repeat && time == repeat->time()) return repeat;

    return KisKeyframeBaseSP();
}

KisKeyframeBaseSP KisKeyframeChannel::activeItemAt(int time) const
{
    const KisKeyframeSP keyframe = activeKeyframeAt(time);
    if (keyframe) return keyframe;

    return activeRepeatAt(time);
}

KisKeyframeBaseSP KisKeyframeChannel::nextItem(const KisKeyframeBase &item) const
{
    const KisKeyframeSP keyframe = nextKeyframe(item.time());

    auto repeatIter = KisCollectionUtils::firstAfter(m_d->repeats, item.time());
    const auto repeat = (repeatIter != m_d->repeats.constEnd()) ? repeatIter.value() : QSharedPointer<KisRepeatFrame>();

    if (keyframe && (!repeat || repeat->time() > keyframe->time())) return keyframe;

    return repeat;
}

KisKeyframeBaseSP KisKeyframeChannel::previousItem(const KisKeyframeBase &item) const
{
    const KisKeyframeSP keyframe = previousKeyframe(item.time());

    auto repeatIter = KisCollectionUtils::lastBefore(m_d->repeats, item.time());
    const auto repeat = (repeatIter != m_d->repeats.constEnd()) ? repeatIter.value() : QSharedPointer<KisRepeatFrame>();
    if (keyframe && (!repeat || repeat->time() > keyframe->time())) return keyframe;

    return repeat;
}

KisRangedKeyframeIterator KisKeyframeChannel::itemsWithin(KisTimeSpan range) const
{
    return KisRangedKeyframeIterator(this, range);
}

KisVisibleKeyframeIterator KisKeyframeChannel::visibleKeyframesFrom(int time) const
{
    return KisVisibleKeyframeIterator(visibleKeyframeAt(time));
}

QList<QSharedPointer<KisAnimationCycle>> KisKeyframeChannel::cycles() const
{
    return m_d->cycles.values();
}

KisTimeSpan KisKeyframeChannel::cycledRangeAt(int time) const
{
    QSharedPointer<KisRepeatFrame> repeat = activeRepeatAt(time);
    if (repeat) return repeat->cycle()->originalRange();

    QSharedPointer<KisAnimationCycle> cycle = cycleAt(time);
    if (cycle) return cycle->originalRange();

    return KisTimeSpan();
}

QSharedPointer<KisAnimationCycle> KisKeyframeChannel::cycleAt(int time) const
{
    if (m_d->cycles.isEmpty()) return QSharedPointer<KisAnimationCycle>();

    const auto it = KisCollectionUtils::lastBeforeOrAt(m_d->cycles, time);
    if (it == m_d->cycles.constEnd()) return QSharedPointer<KisAnimationCycle>();

    const KisKeyframeBaseSP next = nextItem(*it.value()->lastSourceKeyframe());
    if (next && next->time() <= time) return QSharedPointer<KisAnimationCycle>();

    return it.value();
};

QSharedPointer<KisRepeatFrame> KisKeyframeChannel::activeRepeatAt(int time) const
{
    const auto repeat = KisCollectionUtils::lastBeforeOrAt(m_d->repeats, time);
    if (repeat == m_d->repeats.constEnd()) return QSharedPointer<KisRepeatFrame>();

    const KisKeyframeSP lastKeyframe = activeKeyframeAt(time);
    if (lastKeyframe && lastKeyframe->time() > repeat.value()->time()) return QSharedPointer<KisRepeatFrame>();

    return repeat.value();
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

KisFrameSet KisKeyframeChannel::affectedFrames(int time) const
{
    if (m_d->keys.isEmpty()) return KisFrameSet::infiniteFrom(0);

    KeyframesMap::const_iterator active = KisCollectionUtils::lastBeforeOrAt(m_d->keys, time);
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

    KisFrameSet frames;

    QSharedPointer<KisRepeatFrame> repeat = activeRepeatAt(time);

    if (repeat) {
        const KisKeyframeSP original = repeat->getOriginalKeyframeFor(time);
        return affectedFrames(original->time());
    }

    QSharedPointer<KisAnimationCycle> cycle = cycleAt(time);
    if (cycle) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(active != m_d->keys.constEnd() && active.value(), KisFrameSet());
        frames = cycle->instancesWithin(active.value(), KisTimeSpan());
    }

    if (next == m_d->keys.constEnd()) {
        frames |= KisFrameSet::infiniteFrom(from);
    } else {
        frames |= KisFrameSet::between(from, next.key() - 1);
    }

    return frames;
}

KisFrameSet KisKeyframeChannel::identicalFrames(int time, const KisTimeSpan range) const
{
    const QSharedPointer<KisRepeatFrame> repeat = activeRepeatAt(time);
    if (repeat) {
        const KisKeyframeSP original = repeat->getOriginalKeyframeFor(time);
        return identicalFrames(original->time(), range);
    }

    KeyframesMap::const_iterator active = KisCollectionUtils::lastBeforeOrAt(m_d->keys, time);

    const QSharedPointer<KisAnimationCycle> cycle = cycleAt(time);
    if (cycle) {
        return cycle->instancesWithin(active.value(), range);
    }

    if (active != m_d->keys.constEnd() && (active+1) != m_d->keys.constEnd()) {
        if (active->data()->interpolationMode() != KisKeyframe::Constant) {
            return KisFrameSet::between(time, time);
        }
    }

    return affectedFrames(time);
}

bool KisKeyframeChannel::areFramesIdentical(int time1, int time2) const
{
    const KisFrameSet identical = identicalFrames(time1, KisTimeSpan(time2, time2));
    return identical.contains(time2);
}

bool KisKeyframeChannel::isFrameAffectedBy(int targetFrame, int changedFrame) const
{
    const KisFrameSet affected = affectedFrames(changedFrame);
    return affected.contains(targetFrame);
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

    Q_FOREACH (const QSharedPointer<KisAnimationCycle> cycle, m_d->cycles.values()) {
        QDomElement cycleElement = doc.createElement("cycle");
        cycleElement.setAttribute("firstKeyframe", cycle->firstSourceKeyframe()->time());
        cycleElement.setAttribute("lastKeyframe", cycle->lastSourceKeyframe()->time());

        Q_FOREACH (auto repeatWP, cycle->repeats()) {
            const QSharedPointer<KisRepeatFrame> repeat = repeatWP.toStrongRef();
            if (!repeat) continue;

            QDomElement repeatElement = doc.createElement("repeat");
            repeatElement.setAttribute("time", repeat->time());
            cycleElement.appendChild(repeatElement);
        }

        channelElement.appendChild(cycleElement);
    }

    return channelElement;
}

void KisKeyframeChannel::loadXML(const QDomElement &channelNode)
{
    for (QDomElement childNode = channelNode.firstChildElement(); !childNode.isNull(); childNode = childNode.nextSiblingElement()) {
        const QString nodeName = childNode.nodeName().toUpper();
        if (nodeName == "KEYFRAME") {
            const QString keyframeType = childNode.attribute("type", "").toUpper();

            KisKeyframeSP keyframe;
            keyframe = loadKeyframe(childNode);

            KIS_SAFE_ASSERT_RECOVER(keyframe) { continue; }

            if (childNode.hasAttribute("color-label")) {
                keyframe->setColorLabel(childNode.attribute("color-label").toUInt());
            }

            m_d->add(keyframe);
        }
    }

    for (QDomElement childNode = channelNode.firstChildElement(); !childNode.isNull(); childNode = childNode.nextSiblingElement()) {
        if (childNode.nodeName().toUpper() == "CYCLE") {
            QSharedPointer<KisAnimationCycle> cycle = loadCycle(childNode);
            if (cycle) {
                m_d->addCycle(cycle);
            }
        }
    }
}

QSharedPointer<KisAnimationCycle> KisKeyframeChannel::loadCycle(const QDomElement &cycleElement)
{
    const int firstKeyframeTime = cycleElement.attribute("firstKeyframe", "-1").toInt();
    const int lastKeyframeTime = cycleElement.attribute("lastKeyframe", "-1").toInt();

    if (firstKeyframeTime < 0 || lastKeyframeTime <= firstKeyframeTime) {
        qWarning() << "Invalid cycle range: " << firstKeyframeTime << "to" << lastKeyframeTime;
        return nullptr;
    }

    const KisKeyframeSP firstKeyframe = keyframeAt(firstKeyframeTime);
    const KisKeyframeSP lastKeyframe = keyframeAt(lastKeyframeTime);

    if (!firstKeyframe || !lastKeyframe) {
        qWarning() << "Could not find defining keyframes for cycle " << firstKeyframeTime << "to" << lastKeyframeTime;
        return nullptr;
    }

    QSharedPointer<KisAnimationCycle> cycle = toQShared(new KisAnimationCycle(this, firstKeyframe, lastKeyframe));

    for (QDomElement grandChildNode = cycleElement.firstChildElement(); !grandChildNode.isNull(); grandChildNode = grandChildNode.nextSiblingElement()) {
        if (grandChildNode.nodeName().toUpper() == "REPEAT") {
            const int time = grandChildNode.attribute("time").toInt();
            const QSharedPointer<KisRepeatFrame> repeat = toQShared(new KisRepeatFrame(this, time, cycle));
            m_d->add(repeat);
        }
    }

    return cycle;
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

KisDefineCycleCommand * KisKeyframeChannel::createCycle(KisKeyframeSP firstKeyframe, KisKeyframeSP lastKeyframe, KUndo2Command *parentCommand)
{
    const QSharedPointer<KisAnimationCycle> cycle = toQShared(new KisAnimationCycle(this, firstKeyframe, lastKeyframe));
    return new KisDefineCycleCommand(nullptr, cycle, parentCommand);
}

void KisKeyframeChannel::addCycle(QSharedPointer<KisAnimationCycle> cycle)
{
    m_d->addCycle(cycle);
}

KUndo2Command* KisKeyframeChannel::deleteCycle(QSharedPointer<KisAnimationCycle> cycle, KUndo2Command *parentCommand)
{
    KisDefineCycleCommand *defineCycleCommand = new KisDefineCycleCommand(cycle, nullptr, parentCommand);

    // Remove repeats of the cycle
    Q_FOREACH(QWeakPointer<KisRepeatFrame> repeatWP, cycle->repeats()) {
        auto repeat = repeatWP.toStrongRef();
        if (repeat) {
            deleteKeyframe(repeat);
        }
    }

    return defineCycleCommand;
}

void KisKeyframeChannel::removeCycle(QSharedPointer<KisAnimationCycle> cycle)
{
    m_d->removeCycle(cycle);
}

QSharedPointer<KisRepeatFrame> KisKeyframeChannel::addRepeat(QSharedPointer<KisAnimationCycle> cycle, int time, KUndo2Command *parentCommand)
{
    const QSharedPointer<KisRepeatFrame> repeatFrame = toQShared(new KisRepeatFrame(this, time, cycle));
    KUndo2Command *cmd = new KisReplaceKeyframeCommand(this, time, repeatFrame, parentCommand);
    cmd->redo();

    return repeatFrame;
}

void KisKeyframeChannel::requestUpdate(const KisFrameSet &range, const QRect &rect)
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

KisKeyframeBaseSP firstKeyframeInRange(const KisKeyframeChannel *channel, KisTimeSpan range)
{
    KisKeyframeBaseSP active = channel->activeItemAt(range.start());

    if (active) {
        if (range.contains(active->time())) return active;
        if (active->time() < range.start()) {
            const KisKeyframeBaseSP next = channel->nextItem(*active);
            if (next && range.contains(next->time())) return next;
        }
    }

    return KisKeyframeBaseSP();
}

KisRangedKeyframeIterator::KisRangedKeyframeIterator()
{}

KisRangedKeyframeIterator::KisRangedKeyframeIterator(const KisKeyframeChannel *channel, KisKeyframeBaseSP keyframe, KisTimeSpan range)
    : m_channel(channel)
    , m_keyframe(keyframe)
    , m_range(range)
{}

KisRangedKeyframeIterator::KisRangedKeyframeIterator(const KisKeyframeChannel *channel, KisTimeSpan range)
    : KisRangedKeyframeIterator(channel, firstKeyframeInRange(channel, range), range) {}

KisRangedKeyframeIterator& KisRangedKeyframeIterator::operator++()
{
    if (!m_keyframe) return *this;

    m_keyframe = m_channel->nextItem(*m_keyframe);
    if (!m_keyframe || !m_range.contains(m_keyframe->time())) {
        m_keyframe = nullptr;
    }

    return *this;
}

KisRangedKeyframeIterator& KisRangedKeyframeIterator::operator--()
{
    if (!m_keyframe) {
        // One-past-end state: return to last keyframe in range
        const KisKeyframeBaseSP last = m_channel->activeItemAt(m_range.end());
        if (m_range.contains(last->time())) m_keyframe = last;
    } else {
        const KisKeyframeBaseSP previousKeyframe = m_channel->previousItem(*m_keyframe);
        if (previousKeyframe && m_range.contains(previousKeyframe->time())) {
            m_keyframe = previousKeyframe;
        }
    }

    return *this;
}

KisKeyframeBaseSP KisRangedKeyframeIterator::operator*() const
{
    return m_keyframe;
}

KisKeyframeBaseSP KisRangedKeyframeIterator::operator->() const
{
    return m_keyframe;
}

KisRangedKeyframeIterator KisRangedKeyframeIterator::begin() const
{
    return KisRangedKeyframeIterator(m_channel, m_range);
}

KisRangedKeyframeIterator KisRangedKeyframeIterator::end() const
{
    return KisRangedKeyframeIterator(m_channel, nullptr, m_range);
}

bool KisRangedKeyframeIterator::isValid() const
{
    return m_keyframe != nullptr;
}

bool KisRangedKeyframeIterator::operator==(const KisRangedKeyframeIterator &rhs) const
{
    return m_keyframe == rhs.m_keyframe && m_range == rhs.m_range;
}

bool KisRangedKeyframeIterator::operator!=(const KisRangedKeyframeIterator &rhs) const
{
    return !(rhs == *this);
}

KisVisibleKeyframeIterator::KisVisibleKeyframeIterator() = default;

KisVisibleKeyframeIterator::KisVisibleKeyframeIterator(KisKeyframeSP keyframe)
    : m_channel(keyframe->channel())
    , m_keyframe(keyframe)
    , m_time(keyframe->time())
{}

KisVisibleKeyframeIterator& KisVisibleKeyframeIterator::operator--()
{
    const QSharedPointer<KisRepeatFrame> repeat = m_channel->activeRepeatAt(m_time);

    if (repeat) {
        const int time = repeat->previousVisibleFrame(m_time);
        if (time >= 0) {
            m_time = time;
            return *this;
        }
    }

    m_keyframe = m_channel->previousKeyframe(m_keyframe->time());
    if (!m_keyframe) return invalidate();

    m_time = m_keyframe->time();
    return *this;
}

KisVisibleKeyframeIterator& KisVisibleKeyframeIterator::operator++()
{
    const QSharedPointer<KisRepeatFrame> repeat = m_channel->activeRepeatAt(m_time);

    if (repeat) {
        const int time = repeat->nextVisibleFrame(m_time);
        if (time >= 0) {
            m_time = time;
            return *this;
        }
    }

    m_keyframe = m_channel->nextKeyframe(m_keyframe->time());
    if (!m_keyframe) return invalidate();

    m_time = m_keyframe->time();

    return *this;
};

KisKeyframeSP KisVisibleKeyframeIterator::operator*() const
{
    const KisRepeatFrame *repeat = dynamic_cast<KisRepeatFrame*>(m_keyframe.data());

    if (repeat) {
        return repeat->getOriginalKeyframeFor(m_time);
    }

    return m_keyframe;
}

KisKeyframeSP KisVisibleKeyframeIterator::operator->() const
{
    return operator*();
}

bool KisVisibleKeyframeIterator::isValid() const
{
    return m_channel && m_time >= 0;
}

KisVisibleKeyframeIterator& KisVisibleKeyframeIterator::invalidate()
{
    m_channel = nullptr;
    m_keyframe = KisKeyframeSP();
    m_time = -1;

    return *this;
}
