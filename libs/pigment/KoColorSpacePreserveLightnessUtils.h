#ifndef KOCOLORSPACEPRESERVELIGHTNESSUTILS_H
#define KOCOLORSPACEPRESERVELIGHTNESSUTILS_H

#include <KoColorSpaceMaths.h>

template<typename CSTraits>
inline static void fillGrayBrushWithColorPreserveLightnessRGB(quint8 *pixels, const QRgb *brush, quint8 *brushColor, qint32 nPixels) {
    using RGBPixel = typename CSTraits::Pixel;
    using channels_type = typename CSTraits::channels_type;
    static const quint32 pixelSize = CSTraits::pixelSize;

    const RGBPixel *brushColorRGB = reinterpret_cast<const RGBPixel*>(brushColor);

    const float brushColorR = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->red);
    const float brushColorG = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->green);
    const float brushColorB = KoColorSpaceMaths<channels_type, float>::scaleToA(brushColorRGB->blue);
    const float brushColorL = getLightness<HSLType, float>(brushColorR, brushColorG, brushColorB);

    for (; nPixels > 0; --nPixels, pixels += pixelSize, ++brush) {
        RGBPixel *pixelRGB = reinterpret_cast<RGBPixel*>(pixels);

        const float brushMaskL = qRed(*brush) / 255.0f;
        const float lightOffset = 2.0 * brushMaskL - 1.0;
//        const float lightBlend = qAbs(2.0 * brushMaskL - 1.0);
//        const float lightOffset = (brushMaskL - brushColorL) * lightBlend;

        float pixelR = brushColorR;
        float pixelG = brushColorG;
        float pixelB = brushColorB;

        addLightness<HSLType, float>(pixelR, pixelG, pixelB, lightOffset);

        pixelRGB->red = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelR);
        pixelRGB->green = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelG);
        pixelRGB->blue = KoColorSpaceMaths<float, channels_type>::scaleToA(pixelB);
        pixelRGB->alpha = KoColorSpaceMaths<quint8, channels_type>::scaleToA(quint8(qAlpha(*brush)));
    }
}


#endif // KOCOLORSPACEPRESERVELIGHTNESSUTILS_H
