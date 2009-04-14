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
#include <kis_paint_device.h>
#include <kis_random_accessor.h>

#include <GTLCore/PixelDescription.h>
#include <GTLCore/Type.h>
#include <KoColorSpace.h>

GTLCore::PixelDescription csToPD(const KoColorSpace* cs)
{
  std::vector<const GTLCore::Type*> types;
  foreach(KoChannelInfo* channel, cs->channels())
  {
    switch(channel->channelValueType())
    {
      case KoChannelInfo::UINT8:
        types.push_back( GTLCore::Type::UnsignedInteger8 );
        break;
      case KoChannelInfo::INT8:
        types.push_back( GTLCore::Type::Integer8 );
        break;
      case KoChannelInfo::UINT16:
        types.push_back( GTLCore::Type::UnsignedInteger16 );
        break;
      case KoChannelInfo::INT16:
        types.push_back( GTLCore::Type::Integer16 );
        break;
      case KoChannelInfo::UINT32:
        types.push_back( GTLCore::Type::UnsignedInteger32 );
        break;
      case KoChannelInfo::FLOAT16:
        types.push_back( GTLCore::Type::Half );
        break;
      case KoChannelInfo::FLOAT32:
        types.push_back( GTLCore::Type::Float );
        break;
      case KoChannelInfo::FLOAT64:
        types.push_back( GTLCore::Type::Double );
        break;
    }
  }
  return GTLCore::PixelDescription(types);
}

PaintDeviceImage::PaintDeviceImage( KisPaintDeviceSP device) : GTLCore::AbstractImage( csToPD( device->colorSpace()) ), m_device(device)
{
  m_accessor = new KisRandomAccessor(device->createRandomAccessor(0,0));
}

PaintDeviceImage::~PaintDeviceImage()
{
  delete m_accessor;
}

char* PaintDeviceImage::data( int _x, int _y )
{
  m_accessor->moveTo(_x, _y);
  return (char*)(m_accessor->rawData());
}

const char* PaintDeviceImage::data( int _x, int _y ) const
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
