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
#include <KoColorSpaceConstants.h>

class KoCtlChannel
{
public:
    virtual ~KoCtlChannel();
    virtual QString channelValueText(const quint8 *pixel) const = 0;
    virtual QString normalisedChannelValueText(const quint8 *pixel) const = 0;
    virtual quint8 scaleToU8(const quint8 * srcPixel) const = 0;
    virtual void scaleFromU8(quint8 * dstPixel, quint8 value) const = 0;
    virtual quint16 scaleToU16(const quint8 * srcPixel) const = 0;
    virtual float scaleToF32(const quint8 * srcPixel) const = 0;
    virtual void scaleFromF32(quint8 * dstPixel, float value) const = 0;
    virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel) const = 0;
    virtual void multiplyU8(quint8 * pixels, quint8 alpha, qint32 nPixels) const = 0;
    virtual void applyU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const = 0;
    virtual void applyInverseU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const = 0;
};

#include<iostream>
#include<cstdlib>

template<typename _ChannelType_>
class KoCtlChannelImpl : public KoCtlChannel
{
public:
    KoCtlChannelImpl(quint32 _pos, quint32 _pixelSize) : m_pos(_pos), m_pixelSize(_pixelSize) { }
    virtual ~KoCtlChannelImpl() {}
    virtual QString channelValueText(const quint8* pixel) const {
        return QString::number(*channel(pixel));
    }
    virtual QString normalisedChannelValueText(const quint8* pixel) const {
        return QString::number(scaleToF32(pixel));
    }
    virtual quint8 scaleToU8(const quint8* srcPixel) const {
        return KoColorSpaceMaths<_ChannelType_, quint8>::scaleToA(*channel(srcPixel));
    }
    virtual void scaleFromU8(quint8 * dstPixel, quint8 value) const {
        *channel(dstPixel) = KoColorSpaceMaths<quint8, _ChannelType_ >::scaleToA(value);
    }
    virtual quint16 scaleToU16(const quint8* srcPixel) const {
        return KoColorSpaceMaths<_ChannelType_, quint16>::scaleToA(*channel(srcPixel));
    }
    virtual float scaleToF32(const quint8* srcPixel) const {
        return KoColorSpaceMaths<_ChannelType_, float>::scaleToA(*channel(srcPixel));
    }
    virtual void singleChannelPixel(quint8* dstPixel, const quint8 *srcPixel) const {
        *channel(dstPixel) = *channel(srcPixel);
    }
    virtual void scaleFromF32(quint8* dstPixel, float value) const {
        *channel(dstPixel) = KoColorSpaceMaths<float, _ChannelType_ >::scaleToA(value);
    }
    virtual void multiplyU8(quint8 * pixels, quint8 alpha, qint32 nPixels) const {
        _ChannelType_ valpha =  KoColorSpaceMaths<quint8, _ChannelType_>::scaleToA(alpha);

        for (; nPixels > 0; --nPixels, pixels += m_pixelSize) {
            _ChannelType_* alphapixel = channel(pixels);
            *alphapixel = KoColorSpaceMaths<_ChannelType_>::multiply(*alphapixel, valpha);
        }
    }

    virtual void applyU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const {
        for (; nPixels > 0; --nPixels, pixels += m_pixelSize, ++alpha) {
            _ChannelType_ valpha =  KoColorSpaceMaths<quint8, _ChannelType_>::scaleToA(*alpha);
            _ChannelType_* alphapixel = channel(pixels);
            *alphapixel = KoColorSpaceMaths<_ChannelType_>::multiply(*alphapixel, valpha);
        }
    }

    virtual void applyInverseU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const {
        for (; nPixels > 0; --nPixels, pixels += m_pixelSize, ++alpha) {
            _ChannelType_ valpha =  KoColorSpaceMaths<quint8, _ChannelType_>::scaleToA(OPACITY_OPAQUE - *alpha);
            _ChannelType_* alphapixel = channel(pixels);
            *alphapixel = KoColorSpaceMaths<_ChannelType_>::multiply(*alphapixel, valpha);
        }
    }
private:
    inline const _ChannelType_* channel(const quint8* pixel) const {
        return reinterpret_cast<const _ChannelType_*>(pixel + m_pos);
    }
    inline _ChannelType_* channel(quint8* pixel) const {
        return reinterpret_cast<_ChannelType_*>(pixel + m_pos);
    }
private:
    quint32 m_pos;
    quint32 m_pixelSize;
};



#endif
