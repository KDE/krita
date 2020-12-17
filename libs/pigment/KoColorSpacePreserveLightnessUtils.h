/*
 *  SPDX-FileCopyrightText: 2020 Peter Schatz <voronwe13@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KOCOLORSPACEPRESERVELIGHTNESSUTILS_H
#define KOCOLORSPACEPRESERVELIGHTNESSUTILS_H

#include <KoColorSpaceMaths.h>
#include "kis_global.h"

template<typename CSTraits>
inline static void fillGrayBrushWithColorPreserveLightnessRGB(quint8 *pixels, const QRgb *brush, quint8 *brushColor, qreal strength, qint32 nPixels) {
    using RGBPixel = typename CSTraits::Pixel;
        using channels_type = typename CSTraits::channels_type;
        static const quint32 pixelSize = CSTraits::pixelSize;

        const RGBPixel *brushColorRGB = reinterpret_cast<const RGBPixel*>(brushColor);

        const float brushColorR = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->red);
        const float brushColorG = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->green);
        const float brushColorB = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->blue);
        const float brushColorA = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->alpha);

        /**
         * Lightness mixing algorithm is developed by Peter Schatz <voronwe13@gmail.com>
         *
         * We use a formula f(x) where f(0) = 0, f(1) = 1, and f(.5) = z,
         * where z is the lightness of the brush color. This can’t be linear unless
         * the color chosen is also .5. So we use a quadratic equation:
         *
         * f(x) = ax^2 + b^x +c
         * 0,0 -> 0 = a0^2 + b0 + c -> c = 0
         * 1,1 -> 1 = a1^2 +b1 + c -> 1 = a + b + 0 -> a = 1 - b
         * .5,z -> z = a*.5^2 + b*.5 + c -> z =
         *           = a/4 + b/2 + 0 -> z =
         *           = 1/4 - b/4 + b/2 -> z = 1/4 + b/4 -> b = 4z - 1
         *
         * f(x) = (1 - (4z - 1)) * x^2 + (4z - 1) * x
         */

        const float brushColorL = getLightness<HSLType, float>(brushColorR, brushColorG, brushColorB);
        const float lightnessB = 4 * brushColorL - 1;
        const float lightnessA = 1 - lightnessB;

        for (; nPixels > 0; --nPixels, pixels += pixelSize, ++brush) {
            RGBPixel *pixelRGB = reinterpret_cast<RGBPixel*>(pixels);

            float brushMaskL = qRed(*brush) / 255.0f;
            brushMaskL = (brushMaskL - 0.5) * strength + 0.5;
            const float finalLightness = lightnessA * pow2(brushMaskL) + lightnessB * brushMaskL;
            const float finalAlpha = qMin(qAlpha(*brush) / 255.0f, brushColorA);

            float pixelR = brushColorR;
            float pixelG = brushColorG;
            float pixelB = brushColorB;

            setLightness<HSLType, float>(pixelR, pixelG, pixelB, finalLightness);

            pixelRGB->red = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelR);
            pixelRGB->green = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelG);
            pixelRGB->blue = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelB);
            pixelRGB->alpha = KoColorSpaceMaths<quint8, channels_type>::scaleToA(quint8(finalAlpha * 255));
        }
}


#endif // KOCOLORSPACEPRESERVELIGHTNESSUTILS_H
