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

#include <dcopclient.h>

#include "kis_paint_device_iface.h"
#include "kis_colorspace_iface.h"
#include "kis_colorspace.h"

#include "kis_paint_device.h"

KisPaintDeviceIface::KisPaintDeviceIface( KisPaintDevice * parent )
    : DCOPObject("paintdevice")
{
    m_parent = parent;
}

Q_INT32 KisPaintDeviceIface::pixelSize() const
{
    return m_parent->pixelSize();
}

Q_INT32 KisPaintDeviceIface::nChannels() const
{
    return m_parent->nChannels();
}

QByteArray KisPaintDeviceIface::readBytes(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    QByteArray b (w * h * m_parent->pixelSize());

    m_parent->readBytes((Q_UINT8*)b.data(), x, y, w, h);
    return b;
}

void KisPaintDeviceIface::writeBytes(QByteArray bytes, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    m_parent->writeBytes((Q_UINT8*)bytes.data(), x, y, w, h);
}

DCOPRef KisPaintDeviceIface::colorSpace() const
{
    KisColorSpace * cs = m_parent->colorSpace();
    if ( !cs )
        return DCOPRef();
    else
        return DCOPRef( kapp->dcopClient()->appId(),
                        cs->dcopObject()->objId(),
                        "KisColorSpaceIface" );
}

void KisPaintDeviceIface::setColorSpace(DCOPRef)
{
    // XXX: Figure out how to get the correct object from
    //      the dcopref
}
