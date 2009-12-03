/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCtlUtils.h"

#include <GTLCore/Type.h>

GTLCore::PixelDescription createPixelDescription(const KoColorSpace* cs)
{
    QList<KoChannelInfo*> channels = cs->channels();
    std::vector<const GTLCore::Type* > types;
    foreach(KoChannelInfo* info, channels) {
        switch (info->channelValueType()) {
        case KoChannelInfo::UINT8:
            types.push_back(GTLCore::Type::UnsignedInteger8);
            break;
        case KoChannelInfo::UINT16:
            types.push_back(GTLCore::Type::UnsignedInteger16);
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
        case KoChannelInfo::OTHER:
        case KoChannelInfo::FLOAT64:
            Q_ASSERT(false);
            break;
        case KoChannelInfo::INT8:
            types.push_back(GTLCore::Type::Integer8);
            break;
        case KoChannelInfo::INT16:
            types.push_back(GTLCore::Type::Integer16);
            break;
        }
    }
    Q_ASSERT(types.size() == cs->channelCount());
    return GTLCore::PixelDescription(types);
}
