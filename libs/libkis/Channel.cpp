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

#include <QByteArray>
#include <QDataStream>

#include <KoColorModelStandardIds.h>
#include <KoConfig.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <kis_sequential_iterator.h>
#include <kis_layer.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

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
    if (!d->node || !d->channel) return false;
    if (!d->node->inherits("KisLayer")) return false;
    for (int i = 0; i < d->node->colorSpace()->channelCount(); ++i) {
        if (d->node->colorSpace()->channels()[i] == d->channel) {
            KisLayerSP layer = qobject_cast<KisLayer*>(d->node.data());
            QBitArray flags = layer->channelFlags();
            return flags.testBit(i);
        }
    }
    return false;
}

void Channel::setVisible(bool value)
{
    if (!d->node || !d->channel) return;
    if (!d->node->inherits("KisLayer")) return;

    for (int i = 0; i < d->node->colorSpace()->channelCount(); ++i) {
        if (d->node->colorSpace()->channels()[i] == d->channel) {
            QBitArray flags = d->node->colorSpace()->channelFlags(true, true);
            flags.setBit(i, value);
            KisLayerSP layer = qobject_cast<KisLayer*>(d->node.data());
            if (!layer) return;
            layer->setChannelFlags(flags);
            break;
        }
    }

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
    if (!d->node || !d->channel) return QRect();

    QRect rect = d->node->exactBounds();

    KisPaintDeviceSP dev;
    if (d->node->colorSpace()->colorDepthId() == Integer8BitsColorDepthID) {
        dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    }
    else if (d->node->colorSpace()->colorDepthId() ==  Integer16BitsColorDepthID) {
        dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha16());
    }
#ifdef HAVE_OPENEXR
    else if (d->node->colorSpace()->colorDepthId() == Float16BitsColorDepthID) {
        dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha16f());
    }
#endif
    else if (d->node->colorSpace()->colorDepthId() == Float32BitsColorDepthID) {
        dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha32f());
    }

    KisSequentialConstIterator srcIt(d->node->projection(), rect);
    KisSequentialIterator dstIt(dev, rect);

    do {
        const quint8 *srcPtr = srcIt.rawDataConst();
        memcpy(dstIt.rawData(), srcPtr + d->channel->pos(), d->channel->size());

    } while(srcIt.nextPixel() && dstIt.nextPixel());

    if (dev) {
        return dev->exactBounds();
    }

    return QRect();
}

QByteArray Channel::pixelData(const QRect &rect) const
{
    QByteArray ba;

    if (!d->node || !d->channel) return ba;

    QDataStream stream(&ba, QIODevice::WriteOnly);
    KisSequentialConstIterator srcIt(d->node->projection(), rect);

    if (d->node->colorSpace()->colorDepthId() == Integer8BitsColorDepthID) {
        do {
            stream << (quint8) *srcIt.rawDataConst();
        } while(srcIt.nextPixel());
    }
    else if (d->node->colorSpace()->colorDepthId() ==  Integer16BitsColorDepthID) {
        do {
            stream << (quint16) *srcIt.rawDataConst();
        } while(srcIt.nextPixel());
    }
#ifdef HAVE_OPENEXR
    else if (d->node->colorSpace()->colorDepthId() == Float16BitsColorDepthID) {
        do {
            half h = (half)*srcIt.rawDataConst();
            stream << (float)h;
        } while(srcIt.nextPixel());
    }
#endif
    else if (d->node->colorSpace()->colorDepthId() == Float32BitsColorDepthID) {
        do {
            stream << (float) *srcIt.rawDataConst();
        } while(srcIt.nextPixel());

    }

    return ba;
}

void Channel::setPixelData(QByteArray value, const QRect &rect)
{
    if (!d->node || !d->channel || d->node->paintDevice() == 0) return;

    QDataStream stream(&value, QIODevice::ReadOnly);
    KisSequentialIterator dstIt(d->node->paintDevice(), rect);

    if (d->node->colorSpace()->colorDepthId() == Integer8BitsColorDepthID) {
        do {
            quint8 v;
            stream >> v;
            *dstIt.rawData() = v ;
        } while(dstIt.nextPixel());
    }
    else if (d->node->colorSpace()->colorDepthId() ==  Integer16BitsColorDepthID) {
        do {
            quint16 v;
            stream >> v;
            *dstIt.rawData() = v ;
        } while(dstIt.nextPixel());
    }
#ifdef HAVE_OPENEXR
    else if (d->node->colorSpace()->colorDepthId() == Float16BitsColorDepthID) {
        do {
            float f;
            stream >> f;
            half v = f;
            *dstIt.rawData() = v ;
        } while(dstIt.nextPixel());

    }
#endif
    else if (d->node->colorSpace()->colorDepthId() == Float32BitsColorDepthID) {
        do {
            float v;
            stream >> v;
            *dstIt.rawData() = v ;
        } while(dstIt.nextPixel());
    }
}




