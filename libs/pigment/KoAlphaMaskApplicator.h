/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOALPHAMASKAPPLICATOR_H
#define KOALPHAMASKAPPLICATOR_H

#include "KoAlphaMaskApplicatorBase.h"
#include "KoColorSpaceTraits.h"
#include "KoVcMultiArchBuildSupport.h"


template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_,
         Vc::Implementation _impl,
         typename EnableDummyType = void>
struct KoAlphaMaskApplicator : public KoAlphaMaskApplicatorBase
{
    void applyInverseNormedFloatMask(quint8 *pixels,
                                     const float *alpha,
                                     qint32 nPixels) const override {
        KoColorSpaceTrait<
                _channels_type_,
                _channels_nb_,
                _alpha_pos_>::
                applyInverseAlphaNormedFloatMask(pixels, alpha, nPixels);
    }

    void fillInverseAlphaNormedFloatMaskWithColor(quint8 * pixels,
                                                  const float * alpha,
                                                  const quint8 *brushColor,
                                                  qint32 nPixels) const override {
        KoColorSpaceTrait<
                _channels_type_,
                _channels_nb_,
                _alpha_pos_>::
                fillInverseAlphaNormedFloatMaskWithColor(pixels, alpha, brushColor, nPixels);
    }

    void fillGrayBrushWithColor(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const override {
        KoColorSpaceTrait<
                _channels_type_,
                _channels_nb_,
                _alpha_pos_>::
                fillGrayBrushWithColor(dst, brush, brushColor, nPixels);
    }
};

#ifdef HAVE_VC

#include "KoStreamedMath.h"

template<Vc::Implementation _impl>
struct KoAlphaMaskApplicator<
        quint8, 4, 3, _impl,
        typename std::enable_if<_impl != Vc::ScalarImpl>::type> : public KoAlphaMaskApplicatorBase
{
    using uint_v = typename KoStreamedMath<_impl>::uint_v;
    using int_v = typename KoStreamedMath<_impl>::int_v;


    static constexpr int numChannels = 4;
    static constexpr int alphaPos = 3;

    void applyInverseNormedFloatMask(quint8 *pixels,
                                     const float *alpha,
                                     qint32 nPixels) const override
    {
        const int block1 = nPixels / Vc::float_v::size();
        const int block2 = nPixels % Vc::float_v::size();
        const int vectorPixelStride = numChannels * Vc::float_v::size();

        for (int i = 0; i < block1; i++) {
            Vc::float_v maskAlpha(alpha, Vc::Unaligned);

            uint_v data_i;
            data_i.load((const quint32*)pixels, Vc::Unaligned);

            Vc::float_v pixelAlpha = Vc::float_v(int_v(data_i >> 24));
            pixelAlpha *= Vc::float_v(1.0f) - maskAlpha;

            const quint32 colorChannelsMask = 0x00FFFFFF;

            uint_v pixelAlpha_i = uint_v(int_v(Vc::round(pixelAlpha)));
            data_i = (data_i & colorChannelsMask) | (pixelAlpha_i << 24);
            data_i.store((quint32*)pixels, Vc::Unaligned);

            pixels += vectorPixelStride;
            alpha += Vc::float_v::size();
        }

        KoColorSpaceTrait<quint8, 4, 3>::
            applyInverseAlphaNormedFloatMask(pixels, alpha, block2);
        }

    void fillInverseAlphaNormedFloatMaskWithColor(quint8 * pixels,
                                                  const float * alpha,
                                                  const quint8 *brushColor,
                                                  qint32 nPixels) const override {
        const int block1 = nPixels / Vc::float_v::size();
        const int block2 = nPixels % Vc::float_v::size();
        const int vectorPixelStride = numChannels * Vc::float_v::size();
        const uint_v brushColor_i(*reinterpret_cast<const quint32*>(brushColor) & 0x00FFFFFFu);

        for (int i = 0; i < block1; i++) {
            Vc::float_v maskAlpha(alpha, Vc::Unaligned);
            Vc::float_v pixelAlpha = Vc::float_v(255.0f) * (Vc::float_v(1.0f) - maskAlpha);

            uint_v pixelAlpha_i = uint_v(int_v(Vc::round(pixelAlpha)));
            uint_v data_i = brushColor_i | (pixelAlpha_i << 24);
            data_i.store((quint32*)pixels, Vc::Unaligned);

            pixels += vectorPixelStride;
            alpha += Vc::float_v::size();
        }

        KoColorSpaceTrait<quint8, 4, 3>::
            fillInverseAlphaNormedFloatMaskWithColor(pixels, alpha, brushColor, block2);
    }

    static inline uint_v multiply(uint_v a, uint_v b)
    {
        const uint_v c = a * b + 0x80u;
        return ((c >> 8) + c) >> 8;
    }

    void fillGrayBrushWithColor(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const override {
        const int block1 = nPixels / Vc::float_v::size();
        const int block2 = nPixels % Vc::float_v::size();
        const int vectorPixelStride = numChannels * Vc::float_v::size();
        const uint_v brushColor_i(*reinterpret_cast<const quint32*>(brushColor) & 0x00FFFFFFu);

        const uint_v redChannelMask(0xFF);

        for (int i = 0; i < block1; i++) {
            uint_v maskPixels;
            maskPixels.load(reinterpret_cast<const quint32*>(brush), Vc::Unaligned);

            const uint_v pixelAlpha = maskPixels >> 24;
            const uint_v pixelRed = maskPixels & redChannelMask;
            const uint_v pixelAlpha_i = multiply(redChannelMask - pixelRed, pixelAlpha);

            const uint_v data_i = brushColor_i | (pixelAlpha_i << 24);
            data_i.store((quint32*)dst, Vc::Unaligned);

            dst += vectorPixelStride;
            brush += Vc::float_v::size();
        }

        KoColorSpaceTrait<quint8, 4, 3>::
                fillGrayBrushWithColor(dst, brush, brushColor, block2);
    }
};

#endif /* HAVE_VC */

#endif // KOALPHAMASKAPPLICATOR_H
