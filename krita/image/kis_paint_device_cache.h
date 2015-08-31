/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINT_DEVICE_CACHE_H
#define __KIS_PAINT_DEVICE_CACHE_H

#include "kis_lock_free_cache.h"


class KisPaintDeviceCache
{
public:
    KisPaintDeviceCache(KisPaintDevice *paintDevice)
        : m_paintDevice(paintDevice),
          m_exactBoundsCache(paintDevice),
          m_nonDefaultPixelAreaCache(paintDevice),
          m_regionCache(paintDevice)
    {
    }

    KisPaintDeviceCache(const KisPaintDeviceCache &rhs)
        : m_paintDevice(rhs.m_paintDevice),
          m_exactBoundsCache(rhs.m_paintDevice),
          m_nonDefaultPixelAreaCache(rhs.m_paintDevice),
          m_regionCache(rhs.m_paintDevice)
    {
    }

    void setupCache() {
        invalidate();
    }

    void invalidate() {
        m_thumbnailsValid = false;
        m_exactBoundsCache.invalidate();
        m_nonDefaultPixelAreaCache.invalidate();
        m_regionCache.invalidate();
    }

    QRect exactBounds() {
        return m_exactBoundsCache.getValue();
    }

    QRect nonDefaultPixelArea() {
        return m_nonDefaultPixelAreaCache.getValue();
    }

    QRegion region() {
        return m_regionCache.getValue();
    }

    QImage createThumbnail(qint32 w, qint32 h, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) {
        QImage thumbnail;

        if(m_thumbnailsValid) {
            thumbnail = findThumbnail(w, h);
        }
        else {
            m_thumbnails.clear();
            m_thumbnailsValid = true;
        }

        if(thumbnail.isNull()) {
            thumbnail = m_paintDevice->createThumbnail(w, h, QRect(), renderingIntent, conversionFlags);
            cacheThumbnail(w, h, thumbnail);
        }

        Q_ASSERT(!thumbnail.isNull() || m_paintDevice->extent().isEmpty());
        return thumbnail;
    }

private:
    inline QImage findThumbnail(qint32 w, qint32 h) {
        QImage resultImage;
        if (m_thumbnails.contains(w) && m_thumbnails[w].contains(h)) {
            resultImage = m_thumbnails[w][h];
        }
        return resultImage;
    }

    inline void cacheThumbnail(qint32 w, qint32 h, QImage image) {
        m_thumbnails[w][h] = image;
    }

private:
    KisPaintDevice *m_paintDevice;

    struct ExactBoundsCache : KisLockFreeCache<QRect> {
        ExactBoundsCache(KisPaintDevice *paintDevice) : m_paintDevice(paintDevice) {}

        QRect calculateNewValue() const {
            return m_paintDevice->calculateExactBounds(false);
        }
    private:
        KisPaintDevice *m_paintDevice;
    };

    struct NonDefaultPixelCache : KisLockFreeCache<QRect> {
        NonDefaultPixelCache(KisPaintDevice *paintDevice) : m_paintDevice(paintDevice) {}

        QRect calculateNewValue() const {
            return m_paintDevice->calculateExactBounds(true);
        }
    private:
        KisPaintDevice *m_paintDevice;
    };

    struct RegionCache : KisLockFreeCache<QRegion> {
        RegionCache(KisPaintDevice *paintDevice) : m_paintDevice(paintDevice) {}

        QRegion calculateNewValue() const {
            return m_paintDevice->dataManager()->region();
        }
    private:
        KisPaintDevice *m_paintDevice;
    };

    ExactBoundsCache m_exactBoundsCache;
    NonDefaultPixelCache m_nonDefaultPixelAreaCache;
    RegionCache m_regionCache;

    bool m_thumbnailsValid;
    QMap<int, QMap<int, QImage> > m_thumbnails;
};

#endif /* __KIS_PAINT_DEVICE_CACHE_H */
