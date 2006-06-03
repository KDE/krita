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
#include <QColor>
 
#include "KoColor.h"

KoColor::KoColor()
{
    m_data = new quint8[4];
}

KoColor::~KoColor()
{
    delete [] m_data;
}

KoColor::KoColor(const KoColor & rhs)
{
    if (this == &rhs) return;

    memcpy(m_data, rhs.m_data, 4);
}

KoColor & KoColor::operator=(const KoColor & rhs)
{
    memcpy(m_data, rhs.m_data, 4);
    return * this;
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KoColor::toQColor(QColor *c) const
{
    c->setRgb(m_data[0],m_data[1],m_data[2]);
}

void KoColor::toQColor(QColor *c, quint8 *opacity) const
{
    c->setRgb(m_data[0],m_data[1],m_data[2]);
    *opacity = m_data[3];
}

QColor KoColor::toQColor() const
{
    QColor c;
    toQColor(&c);
    return c;
}
