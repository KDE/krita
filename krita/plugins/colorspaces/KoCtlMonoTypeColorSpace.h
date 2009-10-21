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

#ifndef _KO_CTL_MONO_TYPE_COLOR_SPACE_H_
#define _KO_CTL_MONO_TYPE_COLOR_SPACE_H_

#include "KoCtlColorSpace.h"
#include <KoColorSpaceMaths.h>
#include "KoMixColorsOpImpl.h"


template<class _CSTrait>
class KoCtlMonoTypeColorSpace : public KoCtlColorSpace
{
public:
    /**
     * This class is use when creating color space that are defined using the Color Transformation Language,
     * and whose channels are all of the same type.
     */
    KoCtlMonoTypeColorSpace(const QString &id, const QString &name, const KoColorSpace* fallBack, const KoCtlColorProfile* profile) : KoCtlColorSpace(id, name, new KoMixColorsOpImpl< _CSTrait>(), fallBack, profile) {
    }
    ~KoCtlMonoTypeColorSpace() {
    }

    virtual quint32 channelCount() const {
        return _CSTrait::channels_nb;
    }

    virtual quint32 colorChannelCount() const {
        if (_CSTrait::alpha_pos == -1)
            return _CSTrait::channels_nb;
        else
            return _CSTrait::channels_nb - 1;
    }

    virtual quint32 pixelSize() const {
        return _CSTrait::pixelSize;
    }

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const {
        return _CSTrait::channelValueText(pixel, channelIndex);
    }

    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const {
        return _CSTrait::normalisedChannelValueText(pixel, channelIndex);
    }

    virtual void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const {
        return _CSTrait::normalisedChannelsValue(pixel, channels);
    }

    virtual void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const {
        return _CSTrait::fromNormalisedChannelsValue(pixel, values);
    }

    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const {
        typename _CSTrait::channels_type c = _CSTrait::nativeArray(srcPixel)[channelIndex];
        return KoColorSpaceMaths<typename _CSTrait::channels_type, quint8>::scaleToA(c);
    }

    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelIndex) const {
        typename _CSTrait::channels_type c = _CSTrait::nativeArray(srcPixel)[channelIndex];
        return KoColorSpaceMaths<typename _CSTrait::channels_type, quint16>::scaleToA(c);
    }
    virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const {
        _CSTrait::singleChannelPixel(dstPixel, srcPixel, channelIndex);
    }

    virtual quint8 alpha(const quint8 * U8_pixel) const {
        return _CSTrait::alpha(U8_pixel);
    }

    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const {
        _CSTrait::setAlpha(pixels, alpha, nPixels);
    }
    virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const {
        _CSTrait::multiplyAlpha(pixels, alpha, nPixels);
    }

    virtual void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const {
        _CSTrait::applyAlphaU8Mask(pixels, alpha, nPixels);
    }

    virtual void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const {
        _CSTrait::applyInverseAlphaU8Mask(pixels, alpha, nPixels);
    }
    virtual KoID mathToolboxId() const {
        return KoID("Basic");
    }

private:
};

#endif
