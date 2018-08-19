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

#ifndef __VIRTUAL_CHANNEL_INFO_H
#define __VIRTUAL_CHANNEL_INFO_H

#include <KoChannelInfo.h>
class KoColorSpace;


/**
 * This class represents a virtual channel that can have a curve in
 * curves filter. Vitrual channel can be of various types:
 *
 * - REAL --- represents a real color channel of the image,
 *            like R, G, B or A
 *
 * - LIGHTNESS --- lightness virtual channel: represents L channel
 *                 of the image separation itno Lab.
 *
 * - ALL_COLORS --- represents a grouped channel, combining all the
 *                  color channels of the image. E.g. R+G+B of an RGB
 *                  image
 */

class VirtualChannelInfo
{
public:
    enum Type {
        REAL,
        HUE,
        SATURATION,
        LIGHTNESS,
        ALL_COLORS
    };

    VirtualChannelInfo();

    VirtualChannelInfo(Type type, int pixelIndex, KoChannelInfo *realChannelInfo, const KoColorSpace *cs);

    /**
     * \return a pointer to a KoChannelInfo structure *iff* the
     *         channel type is 'REAL'. Returns null of all the
     *         other types.
     */
    KoChannelInfo* channelInfo() const;

    /**
     * Index of this channel in a pixel.
     *
     * \return -1 for all virtual channels.
     */
    int pixelIndex() const;

    Type type() const;
    QString name() const;

    KoChannelInfo::enumChannelValueType valueType() const;
    int channelSize() const;

    bool isAlpha() const;

private:
    Type m_type;
    int m_pixelIndex;
    KoChannelInfo *m_realChannelInfo;

    QString m_nameOverride;
    KoChannelInfo::enumChannelValueType m_valueTypeOverride;
    int m_channelSizeOverride;
};

#endif /* __VIRTUAL_CHANNEL_INFO_H */
