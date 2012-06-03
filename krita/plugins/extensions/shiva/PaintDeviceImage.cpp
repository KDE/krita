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

#include "PaintDeviceImage.h"

#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

#include <kis_paint_device.h>

#include <GTLCore/PixelDescription.h>
#include <GTLCore/Type.h>
#include <GTLCore/Region.h>
#include <KoColorSpaceTraits.h>
#include <kis_random_accessor_ng.h>

GTLCore::PixelDescription csToPD(const KoColorSpace* cs)
{
    std::vector<const GTLCore::Type*> types;
    foreach(KoChannelInfo* channel, cs->channels()) {
        switch (channel->channelValueType()) {
        case KoChannelInfo::UINT8:
            types.push_back(GTLCore::Type::UnsignedInteger8);
            break;
        case KoChannelInfo::INT8:
            types.push_back(GTLCore::Type::Integer8);
            break;
        case KoChannelInfo::UINT16:
            types.push_back(GTLCore::Type::UnsignedInteger16);
            break;
        case KoChannelInfo::INT16:
            types.push_back(GTLCore::Type::Integer16);
            break;
        case KoChannelInfo::UINT32:
            types.push_back(GTLCore::Type::UnsignedInteger32);
            break;
        case KoChannelInfo::FLOAT16:
            types.push_back(GTLCore::Type::Float16);
            break;
        case KoChannelInfo::FLOAT32:
            types.push_back(GTLCore::Type::Float32);
            break;
        case KoChannelInfo::FLOAT64:
            types.push_back(GTLCore::Type::Float64);
            break;
        default:
            errKrita << "Channeltype OTHER encountered";
        }

    }
    GTLCore::PixelDescription pd(types);
    if(cs->colorModelId() == RGBAColorModelID )
    {
        std::vector< std::size_t > positions;
        positions.push_back(KoBgrU16Traits::red_pos);
        positions.push_back(KoBgrU16Traits::green_pos);
        positions.push_back(KoBgrU16Traits::blue_pos);
        positions.push_back(KoBgrU16Traits::alpha_pos);
        pd.setChannelPositions(positions);
    }
    return pd;
}

ConstPaintDeviceImage::ConstPaintDeviceImage(KisPaintDeviceSP device) : GTLCore::AbstractImage(csToPD(device->colorSpace())), m_device(device)
{
    m_accessor = device->createRandomConstAccessorNG(0, 0);
}

ConstPaintDeviceImage::~ConstPaintDeviceImage()
{
}

char* ConstPaintDeviceImage::data(int _x, int _y)
{
    Q_UNUSED(_x);
    Q_UNUSED(_y);
    qFatal("Accessing non const data in a ConstPaintDevice");
    return 0;
}

char* ConstPaintDeviceImage::rawData( int _x, int _y )
{
    Q_UNUSED(_x);
    Q_UNUSED(_y);
    qFatal("Accessing non const data in a ConstPaintDevice");
    return 0;
}

const char* ConstPaintDeviceImage::data(int _x, int _y) const
{
    m_accessor->moveTo(_x, _y);
    return (const char*)(m_accessor->oldRawData());
}

const char* ConstPaintDeviceImage::rawData(int _x, int _y) const
{
    m_accessor->moveTo(_x, _y);
    return (const char*)(m_accessor->oldRawData());
}

GTLCore::RegionI ConstPaintDeviceImage::boundingBox() const
{
  return GTLCore::RegionI(0,0,-1,-1);
}

GTLCore::AbstractImage::ConstIterator* ConstPaintDeviceImage::createIterator() const
{
    return 0;
}

GTLCore::AbstractImage::Iterator* ConstPaintDeviceImage::createIterator()
{
    return 0;
}

PaintDeviceImage::PaintDeviceImage(KisPaintDeviceSP device) : GTLCore::AbstractImage(csToPD(device->colorSpace())), m_device(device)
{
    m_accessor = device->createRandomAccessorNG(0, 0);
}

PaintDeviceImage::~PaintDeviceImage()
{
}

char* PaintDeviceImage::data(int _x, int _y)
{
    m_accessor->moveTo(_x, _y);
    return (char*)(m_accessor->rawData());
}

char* PaintDeviceImage::rawData(int _x, int _y)
{
    m_accessor->moveTo(_x, _y);
    return (char*)(m_accessor->rawData());
}

const char* PaintDeviceImage::data(int _x, int _y) const
{
    m_accessor->moveTo(_x, _y);
    return (const char*)(m_accessor->oldRawData());
}

const char* PaintDeviceImage::rawData(int _x, int _y) const
{
    m_accessor->moveTo(_x, _y);
    return (const char*)(m_accessor->oldRawData());
}

GTLCore::RegionI PaintDeviceImage::boundingBox() const
{
  return GTLCore::RegionI(0,0,-1,-1);
}

GTLCore::AbstractImage::ConstIterator* PaintDeviceImage::createIterator() const
{
    return 0;
}

GTLCore::AbstractImage::Iterator* PaintDeviceImage::createIterator()
{
    return 0;
}
