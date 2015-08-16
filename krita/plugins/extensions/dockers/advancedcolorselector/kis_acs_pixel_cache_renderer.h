/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
         * \p Picker class must provide one method:
         *     - KoColor Picker::colorAt(int x, int y);
         */
        template <class Picker>
        static void render(Picker *picker,
                           const KisDisplayColorConverter *converter,
                           const QRect &pickRect,
                           KisPaintDeviceSP &realPixelCache,
                           QImage &pixelCache,
                           QPoint &pixelCacheOffset)
            {
                const KoColorSpace *cacheColorSpace = converter->paintingColorSpace();
                const int pixelSize = cacheColorSpace->pixelSize();

                if (!realPixelCache || realPixelCache->colorSpace() != cacheColorSpace) {
                    realPixelCache = new KisPaintDevice(cacheColorSpace);
                }

                KoColor color;

                KisSequentialIterator it(realPixelCache, pickRect);

                do {
                    color = picker->colorAt(it.x(), it.y());
                    memcpy(it.rawData(), color.data(), pixelSize);
                } while (it.nextPixel());


                // NOTE: toQImage() function of the converter copies exactBounds() only!
                pixelCache = converter->toQImage(realPixelCache);
                pixelCacheOffset = realPixelCache->exactBounds().topLeft() - pickRect.topLeft();
        }
    };
}

#endif /* __KIS_ACS_PIXEL_CACHE_RENDERER_H */
