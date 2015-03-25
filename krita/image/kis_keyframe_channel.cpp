/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#include <QMap>

struct KisKeyframeChannel::Private
{
    QMap<int, KisKeyframe*> keys;
    KisKeyframeSequence *sequence;
    QString name;
    QString displayName;
};

KisKeyframeChannel::KisKeyframeChannel(const QString &name, const QString &displayName, KisKeyframeSequence *sequence)
    : m_d(new Private)
{
    m_d->sequence = sequence;
    m_d->name = name;
    m_d->displayName = displayName;
}

KisKeyframeChannel::~KisKeyframeChannel()
{
    qDeleteAll(m_d->keys.values());
}

QString KisKeyframeChannel::name() const
{
    return m_d->name;
}

QString KisKeyframeChannel::displayName() const
{
    return m_d->displayName;
}

void KisKeyframeChannel::setKeyframe(int time, const QVariant &value)
{
    deleteKeyframe(time);

    KisKeyframe *keyframe = new KisKeyframe(this, time, value);

    emit sigKeyframeAboutToBeAdded(keyframe);
    m_d->keys.insert(time, keyframe);
    emit sigKeyframeAdded(keyframe);
}

void KisKeyframeChannel::deleteKeyframe(int time)
{
    KisKeyframe *keyframe = m_d->keys.value(time);
    if (!keyframe) return;

    emit sigKeyframeAboutToBeRemoved(keyframe);

    delete keyframe;
    m_d->keys.remove(time);

    emit sigKeyframeRemoved(keyframe);
}

bool KisKeyframeChannel::hasKeyframeAt(int time)
{
    return m_d->keys.contains(time);
}

bool KisKeyframeChannel::moveKeyframe(KisKeyframe *keyframe, int time)
{
    if (m_d->keys.contains(time)) return false;

    emit sigKeyframeAboutToBeMoved(keyframe, time);
    m_d->keys.remove(keyframe->time());

    keyframe->setTime(time);

    m_d->keys.insert(keyframe->time(), keyframe);
    emit sigKeyframeMoved(keyframe);

    return true;
}

QVariant KisKeyframeChannel::getValueAt(int time)
{
    QMap<int, KisKeyframe*>::iterator nextKeyframe = m_d->keys.upperBound(time);

    if (nextKeyframe == m_d->keys.begin()) return QVariant();

    nextKeyframe--;
    return nextKeyframe.value()->value();
}

QList<KisKeyframe*> KisKeyframeChannel::keyframes() const
{
    return m_d->keys.values();
}

KisKeyframeSequence *KisKeyframeChannel::sequence() const
{
    return m_d->sequence;
}

