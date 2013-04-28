/*
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PAINT_DEVICE_WRITER_H
#define KIS_PAINT_DEVICE_WRITER_H

#include <krita_export.h>

class KRITAIMAGE_EXPORT KisPaintDeviceWriter {
public:
    virtual ~KisPaintDeviceWriter() {}
    virtual qint64 write(const QByteArray &data) = 0;
    virtual qint64 write(const char* data, qint64 length) = 0;
};


#endif // KIS_PAINT_DEVICE_WRITER_H
