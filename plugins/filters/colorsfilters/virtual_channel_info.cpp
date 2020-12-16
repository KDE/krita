/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "virtual_channel_info.h"
#include <klocalizedstring.h>

#include <KoColorSpace.h>


VirtualChannelInfo::VirtualChannelInfo()
    : m_type(LIGHTNESS),
      m_pixelIndex(-1),
      m_realChannelInfo(0)
{
}

VirtualChannelInfo::VirtualChannelInfo(Type type,
                                       int pixelIndex,
                                       KoChannelInfo *realChannelInfo,
                                       const KoColorSpace *cs)
    : m_type(type),
      m_pixelIndex(pixelIndex),
      m_realChannelInfo(realChannelInfo)
{
    if (m_type == HUE) {
        m_nameOverride = i18n("Hue");
        m_valueTypeOverride = KoChannelInfo::FLOAT32;
        m_channelSizeOverride = 4;
    } else if (m_type == SATURATION) {
        m_nameOverride = i18n("Saturation");
        m_valueTypeOverride = KoChannelInfo::FLOAT32;
        m_channelSizeOverride = 4;
    } else if (m_type == LIGHTNESS) {
        m_nameOverride = i18nc("Lightness HSI", "Lightness");
        m_valueTypeOverride = KoChannelInfo::FLOAT32;
        m_channelSizeOverride = 4;
    } else if (m_type == ALL_COLORS) {
        m_nameOverride = cs->colorModelId().id();
        m_valueTypeOverride = cs->channels().first()->channelValueType();
        m_channelSizeOverride = cs->channels().first()->size();
    }
}

VirtualChannelInfo::Type VirtualChannelInfo::type() const {
    return m_type;
}

KoChannelInfo* VirtualChannelInfo::channelInfo() const {
    return m_realChannelInfo;
}

QString VirtualChannelInfo::name() const {
    return m_type == REAL ? m_realChannelInfo->name() : m_nameOverride;
}

int VirtualChannelInfo::pixelIndex() const {
    return m_pixelIndex;
}

KoChannelInfo::enumChannelValueType VirtualChannelInfo::valueType() const {
    return m_type == REAL ? m_realChannelInfo->channelValueType() : m_valueTypeOverride;
}

int VirtualChannelInfo::channelSize() const {
    return m_type == REAL ? m_realChannelInfo->size() : m_channelSizeOverride;
}

bool VirtualChannelInfo::isAlpha() const
{
    return m_type == REAL &&
        m_realChannelInfo->channelType() == KoChannelInfo::ALPHA;
}
