/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2007 Emanuele Tamponi <emanuele@valinor.it>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOCOLORSPACEABSTRACT_H
#define KOCOLORSPACEABSTRACT_H

#include <QBitArray>
#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include "KoFallBackColorTransformation.h"
#include "KoLabDarkenColorTransformation.h"
#include "KoMixColorsOpImpl.h"

#include "KoConvolutionOpImpl.h"
#include "KoInvertColorTransformation.h"
#include "KoAlphaMaskApplicatorFactory.h"
#include "KoColorModelStandardIdsUtils.h"

/**
 * This in an implementation of KoColorSpace which can be used as a base for colorspaces with as many
 * different channels of the same type.
 *
 * The template parameters must be a class which inherits KoColorSpaceTrait (or a class with the same signature).
 *
 * SOMETYPE is the type of the channel for instance (quint8, quint32...),
 * SOMENBOFCHANNELS is the number of channels including the alpha channel
 * SOMEALPHAPOS is the position of the alpha channel in the pixel (can be equal to -1 if no alpha channel).
 */
template<class _CSTrait>
class KoColorSpaceAbstract : public KoColorSpace
{
public:
    typedef _CSTrait ColorSpaceTraits;

public:
    KoColorSpaceAbstract(const QString &id, const QString &name)
        : KoColorSpace(id, name, new KoMixColorsOpImpl< _CSTrait>(), new KoConvolutionOpImpl< _CSTrait>()),
          m_alphaMaskApplicator(KoAlphaMaskApplicatorFactory::create(colorDepthIdForChannelType<typename _CSTrait::channels_type>(), _CSTrait::channels_nb, _CSTrait::alpha_pos))
    {
    }

    quint32 colorChannelCount() const override {
        if (_CSTrait::alpha_pos == -1)
            return _CSTrait::channels_nb;
        else
            return _CSTrait::channels_nb - 1;
    }

    quint32 channelCount() const override {
        return _CSTrait::channels_nb;
    }

    quint32 alphaPos() const override {
        return _CSTrait::alpha_pos;
    }


    quint32 pixelSize() const override {
        return _CSTrait::pixelSize;
    }

    QString channelValueText(const quint8 *pixel, quint32 channelIndex) const override {
        return _CSTrait::channelValueText(pixel, channelIndex);
    }

    QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const override {
        return _CSTrait::normalisedChannelValueText(pixel, channelIndex);
    }

    void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const override {
        return _CSTrait::normalisedChannelsValue(pixel, channels);
    }

    void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const override {
        return _CSTrait::fromNormalisedChannelsValue(pixel, values);
    }

    quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const override {
        typename _CSTrait::channels_type c = _CSTrait::nativeArray(srcPixel)[channelIndex];
        return KoColorSpaceMaths<typename _CSTrait::channels_type, quint8>::scaleToA(c);
    }

    void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const override {
        _CSTrait::singleChannelPixel(dstPixel, srcPixel, channelIndex);
    }

    quint8 opacityU8(const quint8 * U8_pixel) const override {
        return _CSTrait::opacityU8(U8_pixel);
    }

    qreal opacityF(const quint8 * U8_pixel) const override {
        return _CSTrait::opacityF(U8_pixel);
    }

    void setOpacity(quint8 * pixels, quint8 alpha, qint32 nPixels) const override {
        _CSTrait::setOpacity(pixels, alpha, nPixels);
    }

    void setOpacity(quint8 * pixels, qreal alpha, qint32 nPixels) const override {
        _CSTrait::setOpacity(pixels, alpha, nPixels);
    }

    void copyOpacityU8(quint8* src, quint8 *dst, qint32 nPixels) const override {
        _CSTrait::copyOpacityU8(src, dst, nPixels);
    }

    void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const override {
        _CSTrait::multiplyAlpha(pixels, alpha, nPixels);
    }

    void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const override {
        _CSTrait::applyAlphaU8Mask(pixels, alpha, nPixels);
    }

    void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const override {
        _CSTrait::applyInverseAlphaU8Mask(pixels, alpha, nPixels);
    }

    void applyAlphaNormedFloatMask(quint8 * pixels, const float * alpha, qint32 nPixels) const override {
        _CSTrait::applyAlphaNormedFloatMask(pixels, alpha, nPixels);
    }

    void applyInverseNormedFloatMask(quint8 * pixels, const float * alpha, qint32 nPixels) const override {
        m_alphaMaskApplicator->applyInverseNormedFloatMask(pixels, alpha, nPixels);
    }

    void fillInverseAlphaNormedFloatMaskWithColor(quint8 * pixels, const float * alpha, const quint8 *brushColor, qint32 nPixels) const override {
        m_alphaMaskApplicator->fillInverseAlphaNormedFloatMaskWithColor(pixels, alpha, brushColor, nPixels);
    }

    void fillGrayBrushWithColor(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const override {
        m_alphaMaskApplicator->fillGrayBrushWithColor(dst, brush, brushColor, nPixels);
    }

    /**
     * By default this does the same as toQColor
     */
    void toQColor16(const quint8 *src, QColor *c) const override {
        this->toQColor(src, c);
    }

    quint8 intensity8(const quint8 * src) const override {
        QColor c;
        const_cast<KoColorSpaceAbstract<_CSTrait> *>(this)->toQColor(src, &c);
        // Integer version of:
        //      static_cast<quint8>(qRound(c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11))
        // The "+ 50" is used for rounding
        return static_cast<quint8>((c.red() * 30 + c.green() * 59 + c.blue() * 11 + 50) / 100);
    }

    qreal intensityF(const quint8 * src) const override {
        QColor c;
        const_cast<KoColorSpaceAbstract<_CSTrait> *>(this)->toQColor16(src, &c);
        return c.redF() * 0.30 + c.greenF() * 0.59 + c.blueF() * 0.11;
    }

    KoColorTransformation* createInvertTransformation() const override {
        return KoInvertColorTransformation::getTransformator(this);
    }

    KoColorTransformation *createDarkenAdjustment(qint32 shade, bool compensate, qreal compensation) const override {
        return new KoFallBackColorTransformation(this, KoColorSpaceRegistry::instance()->lab16(""), new KoLabDarkenColorTransformation<quint16>(shade, compensate, compensation, KoColorSpaceRegistry::instance()->lab16("")));
    }

    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const override
    {
        const int alphaPos = _CSTrait::alpha_pos;

        for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {

            const quint8 *srcPtr = src + pixelIndex * _CSTrait::pixelSize;
            quint8 *dstPtr = dst + pixelIndex * _CSTrait::pixelSize;

            const typename _CSTrait::channels_type *srcPixel = _CSTrait::nativeArray(srcPtr);
            typename _CSTrait::channels_type *dstPixel = _CSTrait::nativeArray(dstPtr);

            typename _CSTrait::channels_type commonChannel = srcPixel[selectedChannelIndex];

            for (uint channelIndex = 0; channelIndex < _CSTrait::channels_nb; ++channelIndex) {

                if (channelIndex != alphaPos) {
                    dstPixel[channelIndex] = commonChannel;
                } else {
                    dstPixel[channelIndex] = srcPixel[channelIndex];
                }
            }
        }
    }

    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const override
    {
        for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
            const quint8 *srcPtr = src + pixelIndex * _CSTrait::pixelSize;
            quint8 *dstPtr = dst + pixelIndex * _CSTrait::pixelSize;

            const typename _CSTrait::channels_type *srcPixel = _CSTrait::nativeArray(srcPtr);
            typename _CSTrait::channels_type *dstPixel = _CSTrait::nativeArray(dstPtr);

            for (uint channelIndex = 0; channelIndex < _CSTrait::channels_nb; ++channelIndex) {
                if (selectedChannels.testBit(channelIndex)) {
                    dstPixel[channelIndex] = srcPixel[channelIndex];
                } else {
                    dstPixel[channelIndex] = _CSTrait::math_trait::zeroValue;
                }
            }
        }
    }

private:
    QScopedPointer<KoAlphaMaskApplicatorBase> m_alphaMaskApplicator;
};

#endif // KOCOLORSPACEABSTRACT_H
