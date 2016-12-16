/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Channel.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

struct Channel::Private {
    Private() {}

    KisNodeSP node;
    KoChannelInfo *channel;

};

Channel::Channel(KisNodeSP node, KoChannelInfo *channel, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->node = node;
    d->channel = channel;
}

Channel::~Channel()
{
    delete d;
}

bool Channel::visible() const
{
    return false;
}

void Channel::setvisible(bool value)
{
}

QString Channel::name() const
{
    return d->channel->name();
}

int Channel::position() const
{
    return d->channel->pos();
}

int Channel::channelSize() const
{
    return d->channel->size();
}

QRect Channel::bounds() const
{
    return QRect();
}

QByteArray Channel::pixelData(const QRect &rect) const
{
    return QByteArray();
}

void Channel::setPixelData(QByteArray value, const QRect &rect)
{

}






