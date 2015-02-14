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

#include "kis_keyframe_sequence.h"

#include <QHash>

struct KisKeyframeSequence::Private
{
    QHash<QString, KisKeyframeChannel*> channels;
};

KisKeyframeSequence::KisKeyframeSequence()
    : m_d(new Private)
{
}

KisKeyframeSequence::~KisKeyframeSequence()
{
    QHash<QString, KisKeyframeChannel*>::const_iterator it = m_d->channels.constBegin();
    while (it != m_d->channels.constEnd()) {
        delete it.value();
        ++it;
    }
}

KisKeyframeChannel * KisKeyframeSequence::createChannel(const QString& name, const QString& displayName)
{
    KisKeyframeChannel *channel = new KisKeyframeChannel(name, displayName);

    m_d->channels.insert(name, channel);

    return channel;
}

KisKeyframeChannel* KisKeyframeSequence::getChannel(const QString& name)
{
    if (m_d->channels.contains(name)) {
        return m_d->channels[name];
    } else {
        return 0;
    }
}
