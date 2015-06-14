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

#include "kis_keyframe.h"
#include "kis_keyframe_channel.h"

QVariant KisKeyframe::value() const
{
    return val;
}

void KisKeyframe::setValue(QVariant value)
{
    val = value;
}

int KisKeyframe::time() const
{
    return t;
}

void KisKeyframe::setTime(int time)
{
    t = time;
}

KisKeyframeChannel *KisKeyframe::channel() const
{
    return ch;
}

bool KisKeyframe::affects(int time) const
{
    return wouldAffect(time, this->time());
}

bool KisKeyframe::wouldAffect(int time, int newTime) const
{
    // TODO: think through corner cases..
    KisKeyframe *nextKey = ch->nextKeyframeAfter(newTime);
    return (newTime <= time && (!nextKey || time < nextKey->time()));
}

#include "kis_keyframe.moc"
