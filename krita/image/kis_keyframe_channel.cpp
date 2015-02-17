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
    QMap<int, QVariant> keys;
};

KisKeyframeChannel::KisKeyframeChannel(const QString& name, const QString& displayName)
    : m_d(new Private)
{
}

void KisKeyframeChannel::setKeyframe(int time, const QVariant& value)
{
    m_d->keys.insert(time, value);
}

void KisKeyframeChannel::deleteKeyframe(int time)
{
    m_d->keys.remove(time);
}

bool KisKeyframeChannel::hasKeyframeAt(int time)
{
    return m_d->keys.contains(time);
}

QVariant KisKeyframeChannel::getValueAt(int time)
{
    QMap<int, QVariant>::iterator nextKeyframe = m_d->keys.upperBound(time);

    if (nextKeyframe == m_d->keys.begin()) return QVariant();

    nextKeyframe--;
    return nextKeyframe.value();
}

QList<int> KisKeyframeChannel::times()
{
    return m_d->keys.keys();
}
