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


#include <QMap>

const KoID KisKeyframeChannel::Content = KoID("content", i18n("Content"));

struct KisKeyframeChannel::Private
{
    KeyframesMap keys;
    KisNodeWSP node;
    KoID id;
};

struct KisKeyframeChannel::InsertFrameCommand : public KUndo2Command
{
    InsertFrameCommand(KisKeyframeChannel *channel, KisKeyframeSP keyframe, bool insert, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_insert(insert)
    {
    }

    void redo() {
        doSwap(m_insert);
    }

    void undo() {
        doSwap(!m_insert);
    }

private:
    void doSwap(bool insert) {
        if (insert) {
            m_channel->insertKeyframeImpl(m_keyframe);
        } else {
            m_channel->deleteKeyframeImpl(m_keyframe);
        }
    }

private:
    KisKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    bool m_insert;
};

struct KisKeyframeChannel::MoveFrameCommand : public KUndo2Command
{
    MoveFrameCommand(KisKeyframeChannel *channel, KisKeyframeSP keyframe, int oldTime, int newTime, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_oldTime(oldTime),
          m_newTime(newTime)
    {
    }

    void redo() {
        m_channel->moveKeyframeImpl(m_keyframe, m_newTime);
    }

    void undo() {
        m_channel->moveKeyframeImpl(m_keyframe, m_oldTime);
    }

private:
    KisKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    int m_oldTime;
    int m_newTime;
};

KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisNodeWSP node)
    : m_d(new Private)
{
    m_d->id = id;
    m_d->node = node;
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

KisNodeWSP KisKeyframeChannel::node() const
{
    return m_d->node;
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

void KisKeyframeChannel::deleteKeyframeImpl(KisKeyframeSP keyframe)
{
    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(keyframe->time());

    emit sigKeyframeAboutToBeRemoved(keyframe.data());
    m_d->keys.remove(keyframe->time());
    emit sigKeyframeRemoved(keyframe.data());

    requestUpdate(range, rect);
}


bool KisKeyframeChannel::deleteKeyframe(KisKeyframeSP keyframe, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    bool result = false;

    if (canDeleteKeyframe(keyframe)) {

        Q_ASSERT(parentCommand);

        KUndo2Command *cmd = new InsertFrameCommand(this, keyframe, false, parentCommand);
        cmd->redo();
        destroyKeyframe(keyframe, parentCommand);

        // FIXME: leak!
        // delete keyframe;

        result = true;
    }

    return result;
}

void KisKeyframeChannel::moveKeyframeImpl(KisKeyframeSP keyframe, int newTime)
{
    KIS_ASSERT_RECOVER_RETURN(keyframe);
    KIS_ASSERT_RECOVER_RETURN(!keyframeAt(newTime));

    KisTimeRange rangeSrc = affectedFrames(keyframe->time());
    QRect rectSrc = affectedRect(keyframe);

    emit sigKeyframeAboutToBeMoved(keyframe.data(), newTime);

    m_d->keys.remove(keyframe->time());
    int oldTime = keyframe->time();
    keyframe->setTime(newTime);
    m_d->keys.insert(newTime, keyframe);

    emit sigKeyframeMoved(keyframe.data(), oldTime);

    KisTimeRange rangeDst = affectedFrames(keyframe->time());
    QRect rectDst = affectedRect(keyframe);

    requestUpdate(rangeSrc, rectSrc);
    requestUpdate(rangeDst, rectDst);
}

bool KisKeyframeChannel::moveKeyframe(KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);

    KisKeyframeSP other = keyframeAt(newTime);
    if (other) return false;

    KUndo2Command *cmd = new MoveFrameCommand(this, keyframe, keyframe->time(), newTime, parentCommand);
    cmd->redo();

    return true;
}

KisKeyframeSP KisKeyframeChannel::copyKeyframe(const KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand)
{
    LAZY_INITIALIZE_PARENT_COMMAND(parentCommand);
    return insertKeyframe(newTime, keyframe, parentCommand);
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
    return activeKeyIterator(time).value();
}

KisKeyframeSP KisKeyframeChannel::firstKeyframe() const
{
    return keyframeAt(0);
}

KisKeyframeSP KisKeyframeChannel::nextKeyframe(KisKeyframeSP keyframe) const
{
    KeyframesMap::iterator i = m_d->keys.find(keyframe->time());
    if (i == m_d->keys.end()) return KisKeyframeSP(0);

    i++;

    if (i == m_d->keys.end()) return KisKeyframeSP(0);
    return i.value();
}

KisKeyframeSP KisKeyframeChannel::previousKeyframe(KisKeyframeSP keyframe) const
{
    KeyframesMap::iterator i = m_d->keys.find(keyframe->time());
    if (i == m_d->keys.begin() || i == m_d->keys.end()) return KisKeyframeSP(0);
    i--;

    return i.value();
}

KisKeyframeSP KisKeyframeChannel::lastKeyframe() const
{
    if (m_d->keys.isEmpty()) return KisKeyframeSP(0);

    return (m_d->keys.end()-1).value();
}

KisTimeRange KisKeyframeChannel::affectedFrames(int time) const
{
    return identicalFrames(time);
}

KisTimeRange KisKeyframeChannel::identicalFrames(int time) const
{
    KeyframesMap::const_iterator active = activeKeyIterator(time);
    KeyframesMap::const_iterator next = active + 1;

    int from;

    if (active == m_d->keys.constBegin()) {
        // First key affects even the frames before it
        from = 0;
    } else {
        from = active.key();
    }

    if (next == m_d->keys.constEnd()) {
        return KisTimeRange::infinite(from);
    } else {
        return KisTimeRange::fromTime(from, next.key() - 1);
    }
}

int KisKeyframeChannel::keyframeCount() const
{
    return m_d->keys.count();
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

    foreach (KisKeyframeSP keyframe, m_d->keys.values()) {
        QDomElement keyframeElement = doc.createElement("keyframe");
        keyframeElement.setAttribute("time", keyframe->time());

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

        m_d->keys.insert(keyframe->time(), keyframe);
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

void KisKeyframeChannel::insertKeyframeImpl(KisKeyframeSP keyframe)
{
    const int time = keyframe->time();

    emit sigKeyframeAboutToBeAdded(keyframe.data());
    m_d->keys.insert(time, keyframe);
    emit sigKeyframeAdded(keyframe.data());

    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(time);
    requestUpdate(range, rect);
}

KisKeyframeSP KisKeyframeChannel::insertKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    KisKeyframeSP keyframe = keyframeAt(time);
    if (keyframe) return keyframe;

    Q_ASSERT(parentCommand);
    keyframe = createKeyframe(time, copySrc, parentCommand);

    KUndo2Command *cmd = new InsertFrameCommand(this, keyframe, true, parentCommand);
    cmd->redo();

    return keyframe;
}

KisKeyframeChannel::KeyframesMap::const_iterator
KisKeyframeChannel::activeKeyIterator(int time) const
{
    KeyframesMap::const_iterator i = const_cast<const KeyframesMap*>(&m_d->keys)->upperBound(time);

    if (i != m_d->keys.constBegin()) i--;

    return i;
}

void KisKeyframeChannel::requestUpdate(const KisTimeRange &range, const QRect &rect)
{
    if (m_d->node) {
        m_d->node->invalidateFrames(range, rect);
    }
}

#include "kis_keyframe_channel.moc"
