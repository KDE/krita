/*
 *  This file is part of the KDE project
 *
 *  Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <kapplication.h>
#include <qbytearray.h>

#include "kis_color_iface.h"

#include "kis_colorspace.h"
#include "kis_color.h"

#include <dcopclient.h>

KisColorIface::KisColorIface( KisColor * parent )
	: DCOPObject("KisColor")
{
	m_parent = parent;
}

QByteArray KisColorIface::data()
{
    QByteArray b;
    b.resize(m_parent->colorSpace().pixelSize());
    memcpy(b.data(), m_parent->data(), b.size());
    return b;
}

DCOPRef KisColorIface::colorSpace()
{
    return DCOPRef( kapp->dcopClient()->appId(),
                    m_parent->colorSpace()->dcopObject()->objId(),
                    "KisColorSpaceIface" );
}

DCOPref KisColorIface::profile()
{
    return DCOPRef( kapp->dcopClient()->appId(),
                    m_parent->profile()->dcopObject()->objId(),
                    "KisProfileIface" );
}

void KisColorIface::convertTo(DCOPRef cs, DCOPRef profile)
{
}

void KisColorIface::setColor(QByteArray data, DCOPRef cs, DCOPRef profile)
{
}

QColor KisColorIface::toQColor()
{
}

Q_UINT8 KisColorIface::opacity()
{
}


