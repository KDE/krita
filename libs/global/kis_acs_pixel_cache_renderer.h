/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ACS_PIXEL_CACHE_RENDERER_H
#define __KIS_ACS_PIXEL_CACHE_RENDERER_H

#include "KoColorSpace.h"
#include "kis_paint_device.h"
#include "kis_iterator_ng.h"
#include "kis_display_color_converter.h"


namespace Acs {

    class PixelCacheRenderer {
    public:
        /**
         * \p Sampler class must provide one method:
         *     - KoColor Sampler::colorAt(float x, float y);
         *
         * How to handle High DPI:
         *  - sampleRect - is in device independent pixels coordinates space
         *     (amount of space on the widget)
         *  - devicePixelRatioF - the amount of UI scaling
         *  - pixelCache and realPixelCache gets the size of
         *     sampleRect.size()*devicePixelRatioF
         *     and sets the device pixel ratio,
         *     and color samplers need to take it into account.
         *  That way you can paint on the cache the same way you'd paint on a low dpi display
         *    and then just use painter->drawImage() and it works.
         */
        template <class Sampler>
        static void render(Sampler *sampler,
                           const KisDisplayColorConverter *converter,
                           const QRect &sampleRect,
                           KisPaintDeviceSP &realPixelCache,
                           QImage &pixelCache,
                           QPoint &pixelCacheOffset,
                           qreal devicePixelRatioF)
            {
                const KoColorSpace *cacheColorSpace = converter->paintingColorSpace();
                const int pixelSize = cacheColorSpace->pixelSize();

                if (!realPixelCache || realPixelCache->colorSpace() != cacheColorSpace) {
                    realPixelCache = new KisPaintDevice(cacheColorSpace);
                }

                KoColor color;

                QRect sampleRectHighDPI = QRect(sampleRect.topLeft(), sampleRect.size()*devicePixelRatioF);
                KisSequentialIterator it(realPixelCache, sampleRectHighDPI);

                while (it.nextPixel()) {
                    color = sampler->colorAt(it.x()/devicePixelRatioF, it.y()/devicePixelRatioF);
                    memcpy(it.rawData(), color.data(), pixelSize);
                }


                // NOTE: toQImage() function of the converter copies exactBounds() only!
                pixelCache = converter->toQImage(realPixelCache);
                pixelCache.setDevicePixelRatio(devicePixelRatioF);
                pixelCacheOffset = realPixelCache->exactBounds().topLeft()/devicePixelRatioF - sampleRect.topLeft();
        }
    };
}

#endif /* __KIS_ACS_PIXEL_CACHE_RENDERER_H */
