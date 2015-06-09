/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "virtual_channel_info.h"
#include <klocale.h>



VirtualChannelInfo::VirtualChannelInfo()
    : m_type(LIGHTNESS),
      m_pixelIndex(-1),
      m_realChannelInfo(0)
{
}

VirtualChannelInfo::VirtualChannelInfo(Type type,
                                       int pixelIndex,
                                       KoChannelInfo *realChannelInfo)
    : m_type(type),
      m_pixelIndex(pixelIndex),
      m_realChannelInfo(realChannelInfo)
{
}

VirtualChannelInfo::Type VirtualChannelInfo::type() const {
    return m_type;
}

KoChannelInfo* VirtualChannelInfo::channelInfo() const {
    return m_realChannelInfo;
}

QString VirtualChannelInfo::name() const {
    return m_type == REAL ? m_realChannelInfo->name() : i18n("Lightness");
}

int VirtualChannelInfo::pixelIndex() const {
    return m_pixelIndex;
}

KoChannelInfo::enumChannelValueType VirtualChannelInfo::valueType() const {
    return m_type == REAL ? m_realChannelInfo->channelValueType() : KoChannelInfo::FLOAT32;
}

int VirtualChannelInfo::channelSize() const {
    return m_type == REAL ? m_realChannelInfo->size() : 4;
}
