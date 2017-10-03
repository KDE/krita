/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include "KoColor.h"

#include <QColor>

#include <QDomDocument>

#include "DebugPigment.h"

#include "KoColorModelStandardIds.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoChannelInfo.h"

const KoColor *KoColor::s_prefab = nullptr;

void KoColor::init()
{
	KIS_ASSERT(s_prefab == nullptr);
	KoColor *prefab = new KoColor(KoColorSpaceRegistry::instance()->rgb16(0));
	prefab->m_colorSpace->fromQColor(Qt::black, prefab->m_data);
	prefab->m_colorSpace->setOpacity(prefab->m_data, OPACITY_OPAQUE_U8, 1);
	s_prefab = prefab;
#ifndef NODEBUG
#ifndef QT_NO_DEBUG
    // warn about rather expensive checks in assertPermanentColorspace().
	qWarning() << "KoColor debug runtime checks are active.";
#endif
#endif
}

KoColor::KoColor(const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memset(m_data, 0, m_size);
}

KoColor::KoColor(const QColor & color, const KoColorSpace * colorSpace)
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);

    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memset(m_data, 0, m_size);

    m_colorSpace->fromQColor(color, m_data);
}

KoColor::KoColor(const quint8 * data, const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);
    Q_ASSERT(data);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memmove(m_data, data, m_size);
}


KoColor::KoColor(const KoColor &src, const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memset(m_data, 0, m_size);

    src.colorSpace()->convertPixelsTo(src.m_data, m_data, colorSpace, 1, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
}

void KoColor::convertTo(const KoColorSpace * cs, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    //dbgPigment <<"Our colormodel:" << d->colorSpace->id().name()
    //      << ", new colormodel: " << cs->id().name() << "\n";

    if (*m_colorSpace == *cs)
        return;

    quint8 data[MAX_PIXEL_SIZE];
    const size_t size = cs->pixelSize();
    Q_ASSERT(size <= MAX_PIXEL_SIZE);
    memset(data, 0, size);

    m_colorSpace->convertPixelsTo(m_data, data, cs, 1, renderingIntent, conversionFlags);

    memcpy(m_data, data, size);
    m_size = size;
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(cs);
}

void KoColor::convertTo(const KoColorSpace * cs)
{
    convertTo(cs,
              KoColorConversionTransformation::internalRenderingIntent(),
              KoColorConversionTransformation::internalConversionFlags());
}

void KoColor::setProfile(const KoColorProfile *profile)
{
    const KoColorSpace *dstColorSpace =
            KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (!dstColorSpace) return;

    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(dstColorSpace);
}

void KoColor::setColor(const quint8 * data, const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);

    const size_t size = colorSpace->pixelSize();
    Q_ASSERT(size <= MAX_PIXEL_SIZE);

    memcpy(m_data, data, size);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KoColor::toQColor(QColor *c) const
{
    Q_ASSERT(c);
    if (m_colorSpace) {
        m_colorSpace->toQColor(m_data, c);
    }
}

QColor KoColor::toQColor() const
{
    QColor c;
    toQColor(&c);
    return c;
}

void KoColor::fromQColor(const QColor& c)
{
    if (m_colorSpace) {
        m_colorSpace->fromQColor(c, m_data);
    }
}

#ifndef NDEBUG
void KoColor::dump() const
{
    dbgPigment <<"KoColor (" << this <<")," << m_colorSpace->id() <<"";
    QList<KoChannelInfo *> channels = m_colorSpace->channels();

    QList<KoChannelInfo *>::const_iterator begin = channels.constBegin();
    QList<KoChannelInfo *>::const_iterator end = channels.constEnd();

    for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it) {
        KoChannelInfo * ch = (*it);
        // XXX: setNum always takes a byte.
        if (ch->size() == sizeof(quint8)) {
            // Byte
            dbgPigment <<"Channel (byte):" << ch->name() <<":" << QString().setNum(m_data[ch->pos()]) <<"";
        } else if (ch->size() == sizeof(quint16)) {
            // Short (may also by an nvidia half)
            dbgPigment <<"Channel (short):" << ch->name() <<":" << QString().setNum(*((const quint16 *)(m_data+ch->pos())))  <<"";
        } else if (ch->size() == sizeof(quint32)) {
            // Integer (may also be float... Find out how to distinguish these!)
            dbgPigment <<"Channel (int):" << ch->name() <<":" << QString().setNum(*((const quint32 *)(m_data+ch->pos())))  <<"";
        }
    }
}
#endif

void KoColor::fromKoColor(const KoColor& src)
{
    src.colorSpace()->convertPixelsTo(src.m_data, m_data, colorSpace(), 1, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
}

const KoColorProfile *KoColor::profile() const
{
    return m_colorSpace->profile();
}

void KoColor::toXML(QDomDocument& doc, QDomElement& colorElt) const
{
    m_colorSpace->colorToXML(m_data, doc, colorElt);
}

void KoColor::setOpacity(quint8 alpha)
{
    m_colorSpace->setOpacity(m_data, alpha, 1);
}
void KoColor::setOpacity(qreal alpha)
{
    m_colorSpace->setOpacity(m_data, alpha, 1);
}
quint8 KoColor::opacityU8() const
{
    return m_colorSpace->opacityU8(m_data);
}
qreal KoColor::opacityF() const
{
    return m_colorSpace->opacityF(m_data);
}

KoColor KoColor::fromXML(const QDomElement& elt, const QString& bitDepthId)
{
    bool ok;
    return fromXML(elt, bitDepthId, &ok);
}

KoColor KoColor::fromXML(const QDomElement& elt, const QString& bitDepthId, bool* ok)
{
    *ok = true;
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
        if (!KoColorSpaceRegistry::instance()->profileByName(profileName)) {
            profileName.clear();
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
        // TODO: Provide a way for colorFromXML() to notify the caller if parsing failed. Currently it returns default values on failure.
        cs->colorFromXML(c.data(), elt);
        return c;
    } else {
        *ok = false;
        return KoColor();
    }
}

QString KoColor::toQString(const KoColor &color)
{
    QStringList ls;
    Q_FOREACH (KoChannelInfo *channel, KoChannelInfo::displayOrderSorted(color.colorSpace()->channels())) {
        int realIndex = KoChannelInfo::displayPositionToChannelIndex(channel->displayPosition(), color.colorSpace()->channels());
        ls << channel->name();
        ls << color.colorSpace()->channelValueText(color.data(), realIndex);
    }
    return ls.join(" ");
}
