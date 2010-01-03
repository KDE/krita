/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOCHANNELINFO_H_
#define KOCHANNELINFO_H_

#include <QtGui/QColor>
#include <QtCore/QString>

/**
 * This class gives some basic information about a channel,
 * that is, one of the components that makes up a particular
 * pixel.
 */
class KoChannelInfo
{
public:
    /// enum to define the type of the channel
    enum enumChannelType {
        COLOR, ///< The channel represents a color
        ALPHA, ///< The channel represents the opacity of a pixel
        SUBSTANCE, ///< The channel represents a real-world substance like pigments or medium
        SUBSTRATE ///< The channel represents a real-world painting substrate like a canvas
    };
    /// enum to define the value of the channel
    enum enumChannelValueType {
        UINT8, ///< use this for an unsigned integer 8bits channel
        UINT16, ///< use this for an integer 16bits channel
        UINT32, ///< use this for an unsigned integer 21bits channel
        FLOAT16, ///< use this for a float 16bits channel
        FLOAT32, ///< use this for a float 32bits channel
        FLOAT64, ///< use this for a float 64bits channel
        INT8, ///< use this for an integer 8bits channel
        INT16, ///< use this for an integer 16bits channel
        OTHER ///< Use this if the channel is neither an integer or a float
    };

public:
    KoChannelInfo() { }
    /**
     * @param name of the channel
     * @param npos position of the channel in the pixel
     * @param channelType type of the channel
     * @param channelValueType type of the numerical data used by the channel
     * @param size number of bytes (not bits) of the channel
     * @param color a color to represent that channel (for instance in an histogram)
     */
    KoChannelInfo(const QString & name, qint32 npos, enumChannelType channelType, enumChannelValueType channelValueType, qint32 size = 1, QColor color = QColor(0, 0, 0))
            : m_name(name), m_pos(npos), m_channelType(channelType), m_channelValueType(channelValueType), m_size(size), m_color(color) { }
public:
    /**
     * User-friendly name for this channel for presentation purposes in the gui
     */
    inline QString name() const {
        return m_name;
    }

    /**
     * returns the position of the first byte of the channel in the pixel
     */
    inline qint32 pos() const {
        return m_pos;
    }

    /**
     * returns the number of bytes this channel takes
     */
    inline qint32 size() const {
        return m_size;
    }

    /**
     * returns the type of the channel
     */
    inline enumChannelType channelType() const {
        return m_channelType;
    }
    /**
     * return the type of the value of the channel (float, uint8 or uint16)
     */
    inline enumChannelValueType channelValueType() const {
        return m_channelValueType;
    }
    /**
     * This is a color that can be used to represent this channel in histograms and so.
     * By default this is black, so keep in mind that many channels might look the same
     */
    inline QColor color() const {
        return m_color;
    }

    /**
     * A channel is less than another channel if its pos is smaller.
     */
    inline bool operator<(const KoChannelInfo & info) {
        return m_pos < info.m_pos;
    }

private:

    QString m_name;
    qint32 m_pos;
    enumChannelType m_channelType;
    enumChannelValueType m_channelValueType;
    qint32 m_size;
    QColor m_color;

};

#endif // KOCHANNELINFO_H_
