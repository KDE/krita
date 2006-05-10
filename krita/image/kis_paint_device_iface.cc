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

qint32 KisPaintDeviceIface::pixelSize() const
{
    return m_parent->pixelSize();
}

qint32 KisPaintDeviceIface::nChannels() const
{
    return m_parent->nChannels();
}

QByteArray KisPaintDeviceIface::readBytes(qint32 x, qint32 y, qint32 w, qint32 h)
{
    QByteArray b;
    b.resize(w * h * m_parent->pixelSize());

    m_parent->readBytes((quint8*)b.data(), x, y, w, h);
    return b;
}

void KisPaintDeviceIface::writeBytes(QByteArray bytes, qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_parent->writeBytes((quint8*)bytes.data(), x, y, w, h);
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
