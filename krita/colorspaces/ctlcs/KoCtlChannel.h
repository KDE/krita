/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_CTL_CHANNEL_H_
#define _KO_CTL_CHANNEL_H_

#include <QtGlobal>
#include <qstring.h>
#include <KoColorSpaceMaths.h>

class KoCtlChannel {
public:
    virtual ~KoCtlChannel();
    virtual QString channelValueText(const quint8 *pixel) const = 0;
    virtual QString normalisedChannelValueText(const quint8 *pixel) const = 0;
    virtual quint8 scaleToU8(const quint8 * srcPixel) const = 0;
    virtual quint16 scaleToU16(const quint8 * srcPixel) const = 0;
    virtual float scaleToF32(const quint8 * srcPixel) const = 0;
    virtual void scaleFromF32(quint8 * dstPixel, float value) const = 0;
    virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel) const = 0;
};

template<typename _ChannelType_>
class KoCtlChannelImpl {
public:
    KoCtlChannelImpl(quint32 _pos) : m_pos(_pos) { }
    virtual ~KoCtlChannelImpl() {}
    virtual QString channelValueText(const quint8* pixel) const
    {
        return QString::number(*channel(pixel));
    }
    virtual QString normalisedChannelValueText(const quint8* pixel) const
    {
        return QString::number( scaleToF32(pixel) );
    }
    virtual quint8 scaleToU8(const quint8* srcPixel) const
    {
        return KoColorSpaceMaths<_ChannelType_, quint8>::scaleToA(*channel(srcPixel) );
    }
    virtual quint16 scaleToU16(const quint8* srcPixel) const
    {
        return KoColorSpaceMaths<_ChannelType_, quint16>::scaleToA(*channel(srcPixel) );
    }
    virtual float scaleToF32(const quint8* srcPixel) const
    {
        return KoColorSpaceMaths<_ChannelType_, float>::scaleToA(*channel(srcPixel) );
    }
    virtual void singleChannelPixel(quint8* dstPixel, const quint8 *srcPixel) const
    {
        *channel(dstPixel) = *channel(srcPixel);
    }
    virtual void scaleFromF32(quint8* dstPixel, float value) const
    {
        *channel(dstPixel) = KoColorSpaceMaths<float, _ChannelType_ >::scaleToA( value );
    }
private:
    inline const _ChannelType_* channel(const quint8* pixel) const
    {
        return reinterpret_cast<const _ChannelType_*>( pixel + m_pos );
    }
    inline _ChannelType_* channel(quint8* pixel) const
    {
        return reinterpret_cast<const _ChannelType_*>( pixel + m_pos );
    }
private:
    quint32 m_pos;
};



#endif
