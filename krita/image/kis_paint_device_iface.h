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
#ifndef _KIS_PAINT_DEVICE_IFACE_H
#define _KIS_PAINT_DEVICE_IFACE_H

#include <dcopref.h>
#include <dcopobject.h>

#include <QString>

class KisPaintDevice;

class KisPaintDeviceIface : virtual public DCOPObject
{
    K_DCOP
public:
    KisPaintDeviceIface( KisPaintDevice * parent );
k_dcop:

    /**
     * Return the number of bytes a pixel takes.
     */
    qint32 pixelSize() const;

    /**
     * Return the number of channels a pixel takes
     */
    qint32 nChannels() const;

    /**
     * Read the bytes representing the rectangle described by x, y, w, h into
     * data. If data is not big enough, Krita will gladly overwrite the rest
     * of your precious memory.
     *
     * Since this is a copy, you need to make sure you have enough memory.
     *
     * Reading from areas not previously initialized will read the default
     * pixel value into data.
     */
    QByteArray readBytes(qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Copy the bytes in data into the rect specified by x, y, w, h. If there
     * data is too small or uninitialized, Krita will happily read parts of
     * memory you never wanted to be read.
     *
     * If the data is written to areas of the paint device not previously initialized,
     * the paint device will grow.
     */
    void writeBytes(QByteArray bytes, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Get the colorspace of this image
     */
    DCOPRef colorSpace() const;

    /**
     * Set the colorspace of this image
     */
    void setColorSpace(DCOPRef colorSpace);


private:

    KisPaintDevice *m_parent;
};

#endif
