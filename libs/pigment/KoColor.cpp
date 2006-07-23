/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
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

KoColor::KoColor()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->colorSpace(KoID("LABA",0),"");
    m_data = new quint8[m_colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());
    m_colorSpace->setAlpha(m_data, OPACITY_OPAQUE, 1);
}

KoColor::~KoColor()
{
    delete [] m_data;
}

KoColor::KoColor(const QColor & color, KoColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    
    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());

    m_colorSpace->fromQColor(color, OPACITY_OPAQUE, m_data);
}


KoColor::KoColor(const QColor & color, quint8 alpha, KoColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());

    m_colorSpace->fromQColor(color, alpha, m_data);
}

KoColor::KoColor(const quint8 * data, KoColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{

    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());
    memmove(m_data, data, colorSpace->pixelSize());
}


KoColor::KoColor(const KoColor &src, KoColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{
    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());

    src.colorSpace()->convertPixelsTo(src.data(), m_data, colorSpace, 1);
}

KoColor::KoColor(const KoColor & rhs)
{
    if (this == &rhs) return;

    m_data = 0;
    m_colorSpace = rhs.colorSpace();
    if(m_colorSpace && rhs.m_data)
    {
        m_data = new quint8[m_colorSpace->pixelSize()];
        memcpy(m_data, rhs.data(), m_colorSpace->pixelSize());
    }
}

KoColor & KoColor::operator=(const KoColor & rhs)
{
    if (this == &rhs) return *this;

    delete [] m_data;
    m_data = 0;
    m_colorSpace = rhs.colorSpace();

    if (rhs.m_colorSpace && rhs.m_data) {
        m_data = new quint8[m_colorSpace->pixelSize()];
        memcpy(m_data, rhs.m_data, m_colorSpace->pixelSize());
    }
    return * this;
}

void KoColor::convertTo(KoColorSpace * cs)
{
    //kDebug(DBG_AREA_CMS) << "Our colormodel: " << m_colorSpace->id().name()
    //      << ", new colormodel: " << cs->id().name() << "\n";

    if (m_colorSpace == cs)
        return;

    quint8 * m_data2 = new quint8[cs->pixelSize()];
    memset(m_data2, 0, cs->pixelSize());

    m_colorSpace->convertPixelsTo(m_data, m_data2, cs, 1);

    delete [] m_data;
    m_data = m_data2;
    m_colorSpace = cs;
}


void KoColor::setColor(quint8 * data, KoColorSpace * colorSpace)
{
    delete [] m_data;
    m_data = new quint8[colorSpace->pixelSize()];
    memcpy(m_data, data, colorSpace->pixelSize());
    m_colorSpace = colorSpace;
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KoColor::toQColor(QColor *c) const
{
    if (m_colorSpace && m_data) {
        // XXX (bsar): There must be a better way, but I'm getting hopelessly confused about constness by now
        KoColorSpace * cs(const_cast<KoColorSpace*>(m_colorSpace));

        cs->toQColor(m_data, c);
    }
}

void KoColor::toQColor(QColor *c, quint8 *opacity) const
{
    if (m_colorSpace && m_data) {
        // XXX (bsar): There must be a better way, but I'm getting hopelessly confused about constness by now
        KoColorSpace * cs(const_cast<KoColorSpace*>(m_colorSpace));
        cs->toQColor(m_data, c, opacity);
    }
}

QColor KoColor::toQColor() const
{
    QColor c;
    toQColor(&c);
    return c;
}

void KoColor::dump() const
{

    //kDebug(DBG_AREA_CMS) << "KoColor (" << this << "), " << m_colorSpace->id().name() << "\n";
    Q3ValueVector<KoChannelInfo *> channels = m_colorSpace->channels();

    Q3ValueVector<KoChannelInfo *>::const_iterator begin = channels.begin();
    Q3ValueVector<KoChannelInfo *>::const_iterator end = channels.end();

    for (Q3ValueVector<KoChannelInfo *>::const_iterator it = begin; it != end; ++it)
    {
        KoChannelInfo * ch = (*it);
        // XXX: setNum always takes a byte.
        if (ch->size() == sizeof(quint8)) {
            // Byte
            //kDebug(DBG_AREA_CMS) << "Channel (byte): " << ch->name() << ": " << QString().setNum(m_data[ch->pos()]) << "\n";
        }
        else if (ch->size() == sizeof(quint16)) {
            // Short (may also by an nvidia half)
            //kDebug(DBG_AREA_CMS) << "Channel (short): " << ch->name() << ": " << QString().setNum(*((const quint16 *)(m_data+ch->pos())))  << "\n";
        }
        else if (ch->size() == sizeof(quint32)) {
            // Integer (may also be float... Find out how to distinguish these!)
            //kDebug(DBG_AREA_CMS) << "Channel (int): " << ch->name() << ": " << QString().setNum(*((const quint32 *)(m_data+ch->pos())))  << "\n";
        }
    }

}
