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

#include "kis_colorspace_iface.h"
#include "kis_colorspace.h"

#include <dcopclient.h>

KisColorSpaceIface::KisColorSpaceIface( KisColorSpace * parent )
	: DCOPObject(parent->id().id().latin1())
{
	m_parent = parent;
}

QByteArray KisColorSpaceIface::invertColor(QByteArray src, Q_INT32 nPixels)
{
    m_parent->invertColor((Q_UINT8*)src.data(), nPixels);
    return src;

}

