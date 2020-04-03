/*
 *  Copyright (c) 2020 Peter Schatz <voronwe13@gmail.com>
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


#ifndef KOCOLORSPACEPRESERVELIGHTNESSUTILS_H
#define KOCOLORSPACEPRESERVELIGHTNESSUTILS_H

#include <KoColorSpaceMaths.h>
#include "kis_global.h"

template<typename CSTraits>
inline static void fillGrayBrushWithColorPreserveLightnessRGB(quint8 *pixels, const QRgb *brush, quint8 *brushColor, qint32 nPixels) {
    using RGBPixel = typename CSTraits::Pixel;
        using channels_type = typename CSTraits::channels_type;
        static const quint32 pixelSize = CSTraits::pixelSize;

        const RGBPixel *brushColorRGB = reinterpret_cast<const RGBPixel*>(brushColor);

        const float brushColorR = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->red);
        const float brushColorG = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->green);
        const float brushColorB = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->blue);

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

            const float brushMaskL = qRed(*brush) / 255.0f;
            const float finalLightness = lightnessA * pow2(brushMaskL) + lightnessB * brushMaskL;

            float pixelR = brushColorR;
            float pixelG = brushColorG;
            float pixelB = brushColorB;

            setLightness<HSLType, float>(pixelR, pixelG, pixelB, finalLightness);

            pixelRGB->red = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelR);
            pixelRGB->green = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelG);
            pixelRGB->blue = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelB);
            pixelRGB->alpha = KoColorSpaceMaths<quint8, channels_type>::scaleToA(quint8(qAlpha(*brush)));
        }
}


#endif // KOCOLORSPACEPRESERVELIGHTNESSUTILS_H
