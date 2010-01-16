/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
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

#include <QDomDocument>

#include "DebugPigment.h"
#include "KoColor.h"


#include "KoColorModelStandardIds.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"


class KoColor::Private
{
public:
    Private() : data(0), colorSpace(0) {}

    ~Private() {
        delete [] data;
    }

    quint8 * data;
    const KoColorSpace * colorSpace;
};

KoColor::KoColor()
        : d(new Private())
{
    d->colorSpace = KoColorSpaceRegistry::instance()->lab16(0);
    d->data = new quint8[d->colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());
    d->colorSpace->setAlpha(d->data, OPACITY_OPAQUE, 1);
}

KoColor::KoColor(const KoColorSpace * colorSpace)
        : d(new Private())
{
    Q_ASSERT(colorSpace);
    d->colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    d->data = new quint8[d->colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());
}


KoColor::~KoColor()
{
    delete d;
}

KoColor::KoColor(const QColor & color, const KoColorSpace * colorSpace)
        : d(new Private())
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    d->colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);

    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());

    d->colorSpace->fromQColor(color, d->data);
}

KoColor::KoColor(const quint8 * data, const KoColorSpace * colorSpace)
        : d(new Private())
{
    Q_ASSERT(colorSpace);
    Q_ASSERT(data);
    d->colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());
    memmove(d->data, data, colorSpace->pixelSize());
}


KoColor::KoColor(const KoColor &src, const KoColorSpace * colorSpace)
        : d(new Private())
{
    Q_ASSERT(colorSpace);
    d->colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    d->data = new quint8[colorSpace->pixelSize()];
    memset(d->data, 0, d->colorSpace->pixelSize());

    src.colorSpace()->convertPixelsTo(src.d->data, d->data, colorSpace, 1);
}

KoColor::KoColor(const KoColor & rhs)
        : d(new Private())
{
    d->colorSpace = rhs.colorSpace();
    Q_ASSERT(*d->colorSpace == *KoColorSpaceRegistry::instance()->permanentColorspace(d->colorSpace));
    if (d->colorSpace && rhs.d->data) {
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
        Q_ASSERT(d->colorSpace == KoColorSpaceRegistry::instance()->permanentColorspace(d->colorSpace)); // here we want to do a check on pointer, since d->colorSpace is supposed to already be a permanent one
        d->data = new quint8[d->colorSpace->pixelSize()];
        memcpy(d->data, rhs.d->data, d->colorSpace->pixelSize());
    }
    return * this;
}

bool KoColor::operator==(const KoColor &other) const
{
    if (!(*colorSpace() == *other.colorSpace()))
        return false;
    return memcmp(d->data, other.d->data, d->colorSpace->pixelSize()) == 0;
}

void KoColor::convertTo(const KoColorSpace * cs)
{
    //dbgPigment <<"Our colormodel:" << d->colorSpace->id().name()
    //      << ", new colormodel: " << cs->id().name() << "\n";

    if (*d->colorSpace == *cs)
        return;

    quint8 * data = new quint8[cs->pixelSize()];
    memset(data, 0, cs->pixelSize());

    d->colorSpace->convertPixelsTo(d->data, data, cs, 1);

    delete [] d->data;
    d->data = data;
    d->colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(cs);
}


void KoColor::setColor(quint8 * data, const KoColorSpace * colorSpace)
{
    Q_ASSERT(data);
    Q_ASSERT(colorSpace);
    delete [] d->data;
    d->data = new quint8[colorSpace->pixelSize()];
    memcpy(d->data, data, colorSpace->pixelSize());
    d->colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KoColor::toQColor(QColor *c) const
{
    Q_ASSERT(c);
    if (d->colorSpace && d->data) {
        d->colorSpace->toQColor(d->data, c);
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

#ifndef NDEBUG
void KoColor::dump() const
{
    //dbgPigment <<"KoColor (" << this <<")," << d->colorSpace->id().name() <<"";
    QList<KoChannelInfo *> channels = d->colorSpace->channels();

    QList<KoChannelInfo *>::const_iterator begin = channels.constBegin();
    QList<KoChannelInfo *>::const_iterator end = channels.constEnd();

    for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it) {
        KoChannelInfo * ch = (*it);
        // XXX: setNum always takes a byte.
        if (ch->size() == sizeof(quint8)) {
            // Byte
            //dbgPigment <<"Channel (byte):" << ch->name() <<":" << QString().setNum(d->data[ch->pos()]) <<"";
        } else if (ch->size() == sizeof(quint16)) {
            // Short (may also by an nvidia half)
            //dbgPigment <<"Channel (short):" << ch->name() <<":" << QString().setNum(*((const quint16 *)(d->data+ch->pos())))  <<"";
        } else if (ch->size() == sizeof(quint32)) {
            // Integer (may also be float... Find out how to distinguish these!)
            //dbgPigment <<"Channel (int):" << ch->name() <<":" << QString().setNum(*((const quint32 *)(d->data+ch->pos())))  <<"";
        }
    }
}
#endif

void KoColor::fromKoColor(const KoColor& src)
{
    src.colorSpace()->convertPixelsTo(src.d->data, d->data, colorSpace(), 1);
}

const KoColorProfile *  KoColor::profile() const
{
    return d->colorSpace->profile();
}

quint8 * KoColor::data()
{
    return d->data;
}

const quint8 * KoColor::data() const
{
    return d->data;
}

const KoColorSpace * KoColor::colorSpace() const
{
    return d->colorSpace;
}

void KoColor::toXML(QDomDocument& doc, QDomElement& colorElt) const
{
    d->colorSpace->colorToXML(d->data, doc, colorElt);
}

void KoColor::setOpacity(quint8 alpha)
{
    d->colorSpace->setAlpha(d->data, alpha, 1);
}
quint8 KoColor::opacity() const
{
    return d->colorSpace->alpha(d->data);
}

KoColor KoColor::fromXML(const QDomElement& elt, const QString & bitDepthId, const QHash<QString, QString> & aliases)
{
    QString modelId;
    if (elt.tagName() == "CMYK") {
        modelId = CMYKAColorModelID.id();
    } else if (elt.tagName() == "RGB") {
        modelId = RGBAColorModelID.id();
    } else if (elt.tagName() == "sRGB") {
        modelId = RGBAColorModelID.id();
    } else if (elt.tagName() == "Lab") {
        modelId = LABAColorModelID.id();
    } else if (elt.tagName() == "XYZ") {
        modelId = XYZAColorModelID.id();
    } else if (elt.tagName() == "Gray") {
        modelId = GrayAColorModelID.id();
    } else if (elt.tagName() == "YCbCr") {
        modelId = YCbCrAColorModelID.id();
    }
    QString profileName;
    if (elt.tagName() != "sRGB") {
        profileName = elt.attribute("space", "");
        if (aliases.contains(profileName)) {
            profileName = aliases.value(profileName);
        }
        if (!KoColorSpaceRegistry::instance()->profileByName(profileName)) {
            profileName = "";
        }
    }
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(modelId, bitDepthId, profileName);
    if (cs == 0) {
        QList<KoID> list =  KoColorSpaceRegistry::instance()->colorDepthList(modelId, KoColorSpaceRegistry::AllColorSpaces);
        if (!list.empty()) {
            cs = KoColorSpaceRegistry::instance()->colorSpace(modelId, list[0].id(), profileName);
        }
    }
    if (cs) {
        KoColor c(cs);
        cs->colorFromXML(c.data(), elt);
        return c;
    } else {
        return KoColor();
    }
}
