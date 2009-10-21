/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PAINT_DEVICE_IMAGE_H_
#define _PAINT_DEVICE_IMAGE_H_

#include <GTLCore/AbstractImage.h>
#include <kis_types.h>

class ConstPaintDeviceImage : public GTLCore::AbstractImage
{
public:
    ConstPaintDeviceImage(KisPaintDeviceSP);
    virtual ~ConstPaintDeviceImage();
    virtual char* data(int _x, int _y);
    virtual const char* data(int _x, int _y) const ;
    virtual ConstIterator* createIterator() const;
    virtual Iterator* createIterator();
private:
    KisPaintDeviceSP m_device;
    KisRandomConstAccessor* m_accessor;
};

class PaintDeviceImage : public GTLCore::AbstractImage
{
public:
    PaintDeviceImage(KisPaintDeviceSP);
    virtual ~PaintDeviceImage();
    virtual char* data(int _x, int _y);
    virtual const char* data(int _x, int _y) const ;
    virtual ConstIterator* createIterator() const;
    virtual Iterator* createIterator();
private:
    KisPaintDeviceSP m_device;
    KisRandomAccessor* m_accessor;
};

#endif
