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

#include <limits>

#include <QColor>
#include <QString>
#include <QList>

/**
 * This class gives some basic information about a channel,
 * that is, one of the components that makes up a particular
 * pixel.
 */
class KoChannelInfo
{
public:
    /**
     * Used to represent a min and max range.
     */
    struct DoubleRange
    {
        public:
            double minVal, maxVal;
        public:
            /// creates an invalid range of 0,0
            DoubleRange(void) : minVal(0), maxVal(0) { }
            /// creates
            DoubleRange(qreal _minVal, qreal _maxVal) : minVal(_minVal), maxVal(_maxVal) { Q_ASSERT(minVal <= maxVal); }
            /// true if this range is usable
            bool isValid(void) const { return minVal < maxVal; }
    };
public:
    /// enum to define the type of the channel
    enum enumChannelType {
        COLOR, ///< The channel represents a color
        ALPHA ///< The channel represents the opacity of a pixel
        //SUBSTANCE, ///< The channel represents a real-world substance like pigments or medium
        //SUBSTRATE ///< The channel represents a real-world painting substrate like a canvas
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
     * @param npos position of the channel in the pixel (in bytes)
     * @param displayPosition the position of the channel in the user-visible order
     * @param channelType type of the channel
     * @param channelValueType type of the numerical data used by the channel
     * @param size number of bytes (not bits) of the channel (if -1, it is deduced from the channelType)
     * @param color a color to represent that channel (for instance in an histogram)
     * @param uiMinMax the UI range
     */
    KoChannelInfo(const QString & name,
                  qint32 npos,
                  qint32 displayPosition,
                  enumChannelType channelType,
                  enumChannelValueType channelValueType,
                  qint32 size = -1,
                  const QColor &color = QColor(0, 0, 0),
                  const DoubleRange &uiMinMax = DoubleRange())
        : m_name(name)
        , m_pos(npos)
        , m_displayPosition(displayPosition)
        , m_channelType(channelType)
        , m_channelValueType(channelValueType)
        , m_size(size)
        , m_color(color)
        , m_uiMinMax(uiMinMax)
    {
        switch(m_channelValueType)
        {
        case UINT8:
        case INT8:
            Q_ASSERT(m_size == -1 || m_size == 1);
            m_size = 1;
            break;
        case UINT16:
        case INT16:
            Q_ASSERT(m_size == -1 || m_size == 2);
            m_size = 2;
            break;
        case UINT32:
            Q_ASSERT(m_size == -1 || m_size == 4);
            m_size = 4;
            break;
        case FLOAT16:
            Q_ASSERT(m_size == -1 || m_size == 2);
            m_size = 2;
            break;
        case FLOAT32:
            Q_ASSERT(m_size == -1 || m_size == 4);
            m_size = 4;
            break;
        case FLOAT64:
            Q_ASSERT(m_size == -1 || m_size == 8);
            m_size = 8;
            break;
        case OTHER:
            Q_ASSERT(m_size != -1);
        }
        if (!uiMinMax.isValid()) {
            switch (m_channelValueType) {
                case UINT8:
                    m_uiMinMax.minVal = std::numeric_limits<quint8>::min();
                    m_uiMinMax.maxVal = std::numeric_limits<quint8>::max();
                    break;
                case INT8:
                    m_uiMinMax.minVal = std::numeric_limits<qint8>::min();
                    m_uiMinMax.maxVal = std::numeric_limits<qint8>::max();
                    break;
                case UINT16:
                    m_uiMinMax.minVal = std::numeric_limits<quint16>::min();
                    m_uiMinMax.maxVal = std::numeric_limits<quint16>::max();
                    break;
                case INT16:
                    m_uiMinMax.minVal = std::numeric_limits<qint16>::min();
                    m_uiMinMax.maxVal = std::numeric_limits<qint16>::max();
                    break;
                case UINT32:
                    m_uiMinMax.minVal = std::numeric_limits<quint32>::min();
                    m_uiMinMax.maxVal = std::numeric_limits<quint32>::max();
                    break;
                default:
                    // assume real otherwise, which is 0..1 by default
                    m_uiMinMax.minVal = 0.0;
                    m_uiMinMax.maxVal = 1.0;
                    break;
            }
        }
        Q_ASSERT(m_uiMinMax.isValid());
    }
public:

    /**
     * converts the display position to the pixel-order index in the channels vector.
     */
    static int displayPositionToChannelIndex(int displayPosition, const QList<KoChannelInfo*> &channels)
    {
        for (int i = 0; i < channels.size(); ++i) {
            if (channels.at(i)->displayPosition() == displayPosition) {
                return i;
            }
        }
        return -1;
    }

    static QList<KoChannelInfo*> displayOrderSorted(const QList<KoChannelInfo*> &channels)
    {
        QList <KoChannelInfo*> sortedChannels;
        for (int i = 0; i < channels.size(); ++i) {
            Q_FOREACH (KoChannelInfo* channel, channels) {
                if (channel->displayPosition() == i) {
                    sortedChannels << channel;
                    break;
                }
            }
        }
        Q_ASSERT(channels.size() == sortedChannels.size());
        return sortedChannels;
    }

    /**
     * User-friendly name for this channel for presentation purposes in the gui
     */
    inline QString name() const {
        return m_name;
    }

    /**
     * @return the position of the first byte of the channel in the pixel
     */
    inline qint32 pos() const {
        return m_pos;
    }

    /**
     * @return the displayPosition of the channel in the pixel
     */
    inline qint32 displayPosition() const {
        return m_displayPosition;
    }

    /**
     * @return the number of bytes this channel takes
     */
    inline qint32 size() const {
        return m_size;
    }

    /**
     * @return the type of the channel
     */
    inline enumChannelType channelType() const {
        return m_channelType;
    }
    /**
     * @return the type of the value of the channel (float, uint8 or uint16)
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

    /**
     * Gets the minimum value that this channel should have.
     * This is suitable for UI use.
     */
    inline double getUIMin(void) const {
        return m_uiMinMax.minVal;
    }

    /**
     * Gets the minimum value that this channel should have.
     * This is suitable for UI use.
     */
    inline double getUIMax(void) const {
        return m_uiMinMax.maxVal;
    }

private:

    QString m_name;
    qint32 m_pos;
    qint32 m_displayPosition;
    enumChannelType m_channelType;
    enumChannelValueType m_channelValueType;
    qint32 m_size;
    QColor m_color;
    DoubleRange m_uiMinMax;

};

#endif // KOCHANNELINFO_H_
