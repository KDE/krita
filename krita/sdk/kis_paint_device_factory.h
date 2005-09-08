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

#ifndef KIS_PAINT_DEVICE_FACTORY_H_
#define KIS_PAINT_DEVICE_FACTORY_H_

class KisAbstractColorSpace;
class QString;
class KisImage;

/**
 * Factory interface for creating paint devices.
 */
class KisPaintDeviceFactory {

public:

    KisPaintDeviceFactory() {};
    virtual ~KisPaintDeviceFactory() {};

public:

    virtual KisPaintDevice * createPaintDevice(KisAbstractColorSpace * colorSpace, const QString& name) = 0;
    virtual KisPaintDevice * createPaintDevice(KisImage * image, const QString& name) = 0;

private:
    
    KisPaintDeviceFactory(const KisPaintDeviceFactory&);
    KisPaintDeviceFactory& operator=(const KisPaintDeviceFactory&);

}

#endif // KIS_PAINT_DEVICE_FACTORY_H_

