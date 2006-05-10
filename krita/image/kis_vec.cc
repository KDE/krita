/*
 *  kis_vec.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#include "kis_vec.h"

KisVector2D& KisVector2D::normalize()
{
    double length, ilength;
  
    length = m_x*m_x + m_y*m_y;
    length = sqrt (length);
  
    if (length > epsilon)
    {
        ilength = 1/length;
        m_x *= ilength;
        m_y *= ilength;
    }
    return *this;
}

KisVector3D& KisVector3D::normalize()
{
    double length, ilength;
  
    length = m_x*m_x + m_y*m_y + m_z*m_z;
    length = sqrt (length);
  
    if (length > epsilon)
    {
        ilength = 1/length;
        m_x *= ilength;
        m_y *= ilength;
        m_z *= ilength;
    }
    return *this;
}

KisVector3D& KisVector3D::crossProduct(const KisVector3D &v)
{
    double x,y,z;
  
    x = m_y*v.m_z - m_z*v.m_y;
    y = m_z*v.m_x - m_x*v.m_z;
    z = m_x*v.m_y - m_y*v.m_x;
    m_x=x; m_y=y; m_z=z;
  
    return *this;
}

