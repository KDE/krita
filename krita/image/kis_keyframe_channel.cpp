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
#include "kis_node.h"

#include <QMap>

struct KisKeyframeChannel::Private
{
    QMap<int, KisKeyframe*> keys;
    KisNodeWSP node;
    KoID id;
};

KisKeyframeChannel::KisKeyframeChannel(const KoID &id, KisNodeWSP node)
    : QObject(node.data()), m_d(new Private)
{
    m_d->id = id;
    m_d->node = node;
}

KisKeyframeChannel::~KisKeyframeChannel()
{
    delete m_d;
}

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

KisKeyframe *KisKeyframeChannel::addKeyframe(int time)
{
    return insertKeyframe(time, 0);
}

bool KisKeyframeChannel::deleteKeyframe(KisKeyframe *keyframe)
{
    if (canDeleteKeyframe(keyframe)) {
        emit sigKeyframeAboutToBeRemoved(keyframe);
        m_d->keys.remove(keyframe->time());
        destroyKeyframe(keyframe);
        emit sigKeyframeRemoved(keyframe);

        delete keyframe;

        return true;
    }

    return false;
}

bool KisKeyframeChannel::moveKeyframe(KisKeyframe *keyframe, int newTime)
{
    KisKeyframe *other = keyframeAt(newTime);
    if (other) return false;

    emit sigKeyframeAboutToBeMoved(keyframe, newTime);

    m_d->keys.remove(keyframe->time());
    int oldTime = keyframe->time();
    keyframe->setTime(newTime);
    m_d->keys.insert(newTime, keyframe);

    emit sigKeyframeMoved(keyframe, oldTime);

    return true;
}

KisKeyframe *KisKeyframeChannel::copyKeyframe(const KisKeyframe *keyframe, int newTime)
{
    return insertKeyframe(newTime, keyframe);
}

KisKeyframe *KisKeyframeChannel::keyframeAt(int time)
{
    QMap<int, KisKeyframe*>::iterator i = m_d->keys.find(time);
    if (i != m_d->keys.end()) {
        return i.value();
    }

    return 0;
}

KisKeyframe *KisKeyframeChannel::activeKeyframeAt(int time) const
{
    return activeKeyIterator(time).value();
}

KisKeyframe *KisKeyframeChannel::nextKeyframeAfter(int time) const
{
    const QMap<int, KisKeyframe *> keys = constKeys();
    QMap<int, KisKeyframe*>::const_iterator i = keys.upperBound(time);

    if (i == keys.end()) return 0;

    return i.value();
}

int KisKeyframeChannel::keyframeCount() const
{
    return m_d->keys.count();
}

QList<KisKeyframe*> KisKeyframeChannel::keyframes() const
{
    return m_d->keys.values();
}

QDomElement KisKeyframeChannel::toXML(QDomDocument doc) const
{
    QDomElement channelElement = doc.createElement("channel");

    channelElement.setAttribute("name", id());

    foreach (KisKeyframe *keyframe, m_d->keys.values()) {
        QDomElement keyframeElement = doc.createElement("keyframe");
        keyframeElement.setAttribute("time", keyframe->time());

        saveKeyframe(keyframe, keyframeElement);

        channelElement.appendChild(keyframeElement);
    }

    return channelElement;
}

void KisKeyframeChannel::loadXML(KoXmlNode channelNode)
{
    for (KoXmlNode keyframeNode = channelNode.firstChild(); !keyframeNode .isNull(); keyframeNode  = keyframeNode .nextSibling()) {
        if (keyframeNode.nodeName().toUpper() != "KEYFRAME") continue;

        KisKeyframe *keyframe = loadKeyframe(keyframeNode);

        m_d->keys.insert(keyframe->time(), keyframe);
    }
}

QMap<int, KisKeyframe *> KisKeyframeChannel::keys()
{
    return m_d->keys;
}

const QMap<int, KisKeyframe *> KisKeyframeChannel::constKeys() const
{
    return m_d->keys;
}

KisKeyframe * KisKeyframeChannel::insertKeyframe(int time, const KisKeyframe *copySrc)
{
    KisKeyframe *keyframe = keyframeAt(time);
    if (keyframe) return keyframe;

    keyframe = createKeyframe(time, copySrc);

    emit sigKeyframeAboutToBeAdded(keyframe);
    m_d->keys.insert(time, keyframe);
    emit sigKeyframeAdded(keyframe);

    return keyframe;
}

QMap<int, KisKeyframe*>::const_iterator KisKeyframeChannel::activeKeyIterator(int time) const
{
    const QMap<int, KisKeyframe *> keys = constKeys();
    QMap<int, KisKeyframe*>::const_iterator i = keys.upperBound(time);

    if (i != keys.begin()) i--;

    return i;
}

#include "kis_keyframe_channel.moc"
