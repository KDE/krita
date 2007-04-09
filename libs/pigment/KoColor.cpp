/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include <QColor>

#include "kdebug.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

class KoColor::Private {
public:
    Private() : data(0), colorSpace(0) {}
    ~Private() {
        delete [] data;
    }
    quint8 * data;
    KoColorSpace * colorSpace;
};

KoColor::KoColor()
    : d(new Private())
{
    d->colorSpace = KoColorSpaceRegistry::instance()->colorSpace("LABA",0);
    d->data = new quint8[d->colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());
    d->colorSpace->setAlpha(d->data, OPACITY_OPAQUE, 1);
}
KoColor::KoColor(KoColorSpace * colorSpace)
    : d(new Private())
{
    d->colorSpace = colorSpace;
    d->data = new quint8[d->colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());
}


KoColor::~KoColor()
{
    delete d;
}

KoColor::KoColor(const QColor & color, KoColorSpace * colorSpace)
    : d(new Private())
{
    d->colorSpace = colorSpace;
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);

    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());

    d->colorSpace->fromQColor(color, OPACITY_OPAQUE, d->data);
}


KoColor::KoColor(const QColor & color, quint8 alpha, KoColorSpace * colorSpace)
    : d(new Private())
{
    d->colorSpace = colorSpace;
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());

    d->colorSpace->fromQColor(color, alpha, d->data);
}

KoColor::KoColor(const quint8 * data, KoColorSpace * colorSpace)
    : d(new Private())
{
    d->colorSpace = colorSpace;
    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());
    memmove(d->data, data, colorSpace->pixelSize());
}


KoColor::KoColor(const KoColor &src, KoColorSpace * colorSpace)
    : d(new Private())
{
    d->colorSpace = colorSpace;
    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());

    src.colorSpace()->convertPixelsTo(src.d->data, d->data, colorSpace, 1);
}

KoColor::KoColor(const KoColor & rhs)
    : d(new Private())
{
    d->colorSpace = rhs.colorSpace();
    if(d->colorSpace && rhs.d->data)
    {
        d->data = new quint8[d->colorSpace->pixelSize()];
        memcpy(d->data, rhs.d->data, d->colorSpace->pixelSize());
    }
}

KoColor & KoColor::operator=(const KoColor & rhs)
{
    if (this == &rhs) return *this;

    delete [] d->data;
    d->data = 0;
    d->colorSpace = rhs.colorSpace();

    if (rhs.d->colorSpace && rhs.d->data) {
        d->data = new quint8[d->colorSpace->pixelSize()];
        memcpy(d->data, rhs.d->data, d->colorSpace->pixelSize());
    }
    return * this;
}

void KoColor::convertTo(KoColorSpace * cs)
{
    //kDebug(DBG_AREA_CMS) << "Our colormodel: " << d->colorSpace->id().name()
    //      << ", new colormodel: " << cs->id().name() << "\n";

    if (d->colorSpace == cs)
        return;

    quint8 * data = new quint8[cs->pixelSize()];
    memset(data, 0, cs->pixelSize());

    d->colorSpace->convertPixelsTo(d->data, data, cs, 1);

    delete [] d->data;
    d->data = data;
    d->colorSpace = cs;
}


void KoColor::setColor(quint8 * data, KoColorSpace * colorSpace)
{
    delete [] d->data;
    d->data = new quint8[colorSpace->pixelSize()];
    memcpy(d->data, data, colorSpace->pixelSize());
    d->colorSpace = colorSpace;
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KoColor::toQColor(QColor *c) const
{
    if (d->colorSpace && d->data) {
        d->colorSpace->toQColor(d->data, c);
    }
}

void KoColor::toQColor(QColor *c, quint8 *opacity) const
{
    if (d->colorSpace && d->data) {
        d->colorSpace->toQColor(d->data, c, opacity);
    }
}

QColor KoColor::toQColor() const
{
    QColor c;
    toQColor(&c);
    return c;
}

void KoColor::fromQColor(const QColor& c) const
{
    if (d->colorSpace && d->data) {
        d->colorSpace->fromQColor(c, d->data);
    }
}

void KoColor::fromQColor(const QColor& c, quint8 opacity) const
{
    if (d->colorSpace && d->data) {
        d->colorSpace->fromQColor(c, opacity, d->data);
    }
}

#ifndef NDEBUG
void KoColor::dump() const
{
    //kDebug(DBG_AREA_CMS) << "KoColor (" << this << "), " << d->colorSpace->id().name() << "\n";
    QList<KoChannelInfo *> channels = d->colorSpace->channels();

    QList<KoChannelInfo *>::const_iterator begin = channels.begin();
    QList<KoChannelInfo *>::const_iterator end = channels.end();

    for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it)
    {
        KoChannelInfo * ch = (*it);
        // XXX: setNum always takes a byte.
        if (ch->size() == sizeof(quint8)) {
            // Byte
            //kDebug(DBG_AREA_CMS) << "Channel (byte): " << ch->name() << ": " << QString().setNum(d->data[ch->pos()]) << "\n";
        }
        else if (ch->size() == sizeof(quint16)) {
            // Short (may also by an nvidia half)
            //kDebug(DBG_AREA_CMS) << "Channel (short): " << ch->name() << ": " << QString().setNum(*((const quint16 *)(d->data+ch->pos())))  << "\n";
        }
        else if (ch->size() == sizeof(quint32)) {
            // Integer (may also be float... Find out how to distinguish these!)
            //kDebug(DBG_AREA_CMS) << "Channel (int): " << ch->name() << ": " << QString().setNum(*((const quint32 *)(d->data+ch->pos())))  << "\n";
        }
    }
}
#endif

void KoColor::fromKoColor(const KoColor& src)
{
    src.colorSpace()->convertPixelsTo(src.d->data, d->data, colorSpace(), 1);
}

KoColorProfile *  KoColor::profile() const
{
    return d->colorSpace->profile();
}

quint8 * KoColor::data() const {
    return d->data;
}

KoColorSpace * KoColor::colorSpace() const {
    return d->colorSpace;
}

