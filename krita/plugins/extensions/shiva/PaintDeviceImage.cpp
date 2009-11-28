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
#include <kis_random_accessor.h>

#include <GTLCore/PixelDescription.h>
#include <GTLCore/Type.h>
#include <OpenShiva/Version.h>
#include <KoColorSpaceTraits.h>

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
#if OPENSHIVA_VERSION_MAJOR == 0 && OPENSHIVA_VERSION_MINOR == 9 && OPENSHIVA_VERSION_REVISION < 13
            types.push_back(GTLCore::Type::Double);
#else
            types.push_back(GTLCore::Type::Float64);
#endif
            break;
        }
    }
    GTLCore::PixelDescription pd(types);
    if(cs->colorModelId() == RGBAColorModelID )
    {
        std::vector< std::size_t > positions;
        positions.push_back(KoRgbU16Traits::red_pos);
        positions.push_back(KoRgbU16Traits::green_pos);
        positions.push_back(KoRgbU16Traits::blue_pos);
        positions.push_back(KoRgbU16Traits::alpha_pos);
        pd.setChannelPositions(positions);
    }
    return pd;
}

ConstPaintDeviceImage::ConstPaintDeviceImage(KisPaintDeviceSP device) : GTLCore::AbstractImage(csToPD(device->colorSpace())), m_device(device)
{
    m_accessor = new KisRandomConstAccessor(device->createRandomConstAccessor(0, 0));
}

ConstPaintDeviceImage::~ConstPaintDeviceImage()
{
    delete m_accessor;
}

char* ConstPaintDeviceImage::data(int _x, int _y)
{
    // TODO should return 0, when http://bugs.opengtl.org/index.php?do=details&task_id=24 is fixed
    m_accessor->moveTo(_x, _y);
    return const_cast<char*>((const char*)(m_accessor->oldRawData()));
}

const char* ConstPaintDeviceImage::data(int _x, int _y) const
{
    m_accessor->moveTo(_x, _y);
    return (const char*)(m_accessor->oldRawData());
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
    m_accessor = new KisRandomAccessor(device->createRandomAccessor(0, 0));
}

PaintDeviceImage::~PaintDeviceImage()
{
    delete m_accessor;
}

char* PaintDeviceImage::data(int _x, int _y)
{
    m_accessor->moveTo(_x, _y);
    return (char*)(m_accessor->rawData());
}

const char* PaintDeviceImage::data(int _x, int _y) const
{
    m_accessor->moveTo(_x, _y);
    return (const char*)(m_accessor->oldRawData());
}

GTLCore::AbstractImage::ConstIterator* PaintDeviceImage::createIterator() const
{
    return 0;
}

GTLCore::AbstractImage::Iterator* PaintDeviceImage::createIterator()
{
    return 0;
}
