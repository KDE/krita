/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <qcolor.h>
 
#include "kdebug.h"
#include "kis_debug_areas.h"
#include "kis_color.h"
#include "kis_profile.h"
#include "kis_colorspace.h"
#include "kis_colorspace_factory_registry.h"

KisColor::KisColor()
{
    m_data = 0;
    m_colorSpace = 0;
}

KisColor::~KisColor()
{
    delete [] m_data;
}

KisColor::KisColor(const QColor & color, KisColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    
    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());

    m_colorSpace->fromQColor(color, OPACITY_OPAQUE, m_data);
}


KisColor::KisColor(const QColor & color, quint8 alpha, KisColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());

    m_colorSpace->fromQColor(color, alpha, m_data);
}

KisColor::KisColor(const quint8 * data, KisColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{

    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());
    memmove(m_data, data, colorSpace->pixelSize());
}


KisColor::KisColor(const KisColor &src, KisColorSpace * colorSpace)
    : m_colorSpace(colorSpace)
{
    m_data = new quint8[colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());

    src.colorSpace()->convertPixelsTo(src.data(), m_data, colorSpace, 1);
}

KisColor::KisColor(const KisColor & rhs)
{
    if (this == &rhs) return;

    m_colorSpace = rhs.colorSpace();
    m_data = new quint8[m_colorSpace->pixelSize()];
    memset(m_data, 0, m_colorSpace->pixelSize());
    memcpy(m_data, rhs.data(), m_colorSpace->pixelSize());
}

KisColor & KisColor::operator=(const KisColor & rhs)
{
    delete [] m_data;
    m_data = 0;
    m_colorSpace = rhs.colorSpace();

    if (rhs.m_colorSpace && rhs.m_data) {
        m_data = new quint8[m_colorSpace->pixelSize()];
        memcpy(m_data, rhs.m_data, m_colorSpace->pixelSize());
    }
    return * this;
}

void KisColor::convertTo(KisColorSpace * cs)
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


void KisColor::setColor(quint8 * data, KisColorSpace * colorSpace)
{
    delete [] m_data;
    m_data = new quint8[colorSpace->pixelSize()];
    memcpy(m_data, data, colorSpace->pixelSize());
    m_colorSpace = colorSpace;
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KisColor::toQColor(QColor *c) const
{
    if (m_colorSpace && m_data) {
        // XXX (bsar): There must be a better way, but I'm getting hopelessly confused about constness by now
        KisColorSpace * cs(const_cast<KisColorSpace*>(m_colorSpace));

        cs->toQColor(m_data, c);
    }
}

void KisColor::toQColor(QColor *c, quint8 *opacity) const
{
    if (m_colorSpace && m_data) {
        // XXX (bsar): There must be a better way, but I'm getting hopelessly confused about constness by now
        KisColorSpace * cs(const_cast<KisColorSpace*>(m_colorSpace));
        cs->toQColor(m_data, c, opacity);
    }
}

QColor KisColor::toQColor() const
{
    QColor c;
    toQColor(&c);
    return c;
}

void KisColor::dump() const
{

    //kDebug(DBG_AREA_CMS) << "KisColor (" << this << "), " << m_colorSpace->id().name() << "\n";
    Q3ValueVector<KisChannelInfo *> channels = m_colorSpace->channels();

    Q3ValueVector<KisChannelInfo *>::const_iterator begin = channels.begin();
    Q3ValueVector<KisChannelInfo *>::const_iterator end = channels.end();

    for (Q3ValueVector<KisChannelInfo *>::const_iterator it = begin; it != end; ++it)
    {
        KisChannelInfo * ch = (*it);
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
