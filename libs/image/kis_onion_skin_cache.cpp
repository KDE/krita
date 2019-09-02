/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_onion_skin_cache.h"

#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>


#include "kis_paint_device.h"
#include "kis_onion_skin_compositor.h"
#include "kis_default_bounds.h"
#include "kis_image.h"

#include "kis_raster_keyframe_channel.h"


struct KisOnionSkinCache::Private
{
    KisPaintDeviceSP cachedProjection;

    int cacheTime = 0;
    int cacheConfigSeqNo = 0;
    int framesHash = 0;
    QReadWriteLock lock;

    bool checkCacheValid(KisPaintDeviceSP source, KisOnionSkinCompositor *compositor) {
        const KisRasterKeyframeChannel *keyframes = source->keyframeChannel();

        const int time = source->defaultBounds()->currentTime();
        const int seqNo = compositor->configSeqNo();
        const int hash = keyframes->framesHash();

        return time == cacheTime && cacheConfigSeqNo == seqNo && framesHash == hash;
    }

    void updateCacheMetrics(KisPaintDeviceSP source, KisOnionSkinCompositor *compositor) {
        const KisRasterKeyframeChannel *keyframes = source->keyframeChannel();

        const int time = source->defaultBounds()->currentTime();
        const int seqNo = compositor->configSeqNo();
        const int hash = keyframes->framesHash();

        cacheTime = time;
        cacheConfigSeqNo = seqNo;
        framesHash = hash;
    }
};

KisOnionSkinCache::KisOnionSkinCache()
    : m_d(new Private)
{
}

KisOnionSkinCache::~KisOnionSkinCache()
{
}

KisPaintDeviceSP KisOnionSkinCache::projection(KisPaintDeviceSP source)
{
    KisOnionSkinCompositor *compositor = KisOnionSkinCompositor::instance();

    KisPaintDeviceSP cachedProjection;

    QReadLocker readLocker(&m_d->lock);
    cachedProjection = m_d->cachedProjection;

    if (!cachedProjection || !m_d->checkCacheValid(source, compositor)) {

        readLocker.unlock();
        QWriteLocker writeLocker(&m_d->lock);
        cachedProjection = m_d->cachedProjection;
        if (!cachedProjection || !m_d->checkCacheValid(source, compositor)) {

            if (!cachedProjection) {
                cachedProjection = new KisPaintDevice(source->colorSpace());
            } else {
                cachedProjection->setDefaultBounds(new KisDefaultBounds());
                cachedProjection->clear();
            }

            const QRect extent = compositor->calculateExtent(source);
            compositor->composite(source, cachedProjection, extent);

            cachedProjection->setDefaultBounds(source->defaultBounds());

            /**
             * It might happen that the lod planes has already been
             * generated for all the devices, so we should cold-init them
             * for the onion skins.
             */
            const int lod = source->defaultBounds()->currentLevelOfDetail();
            if (lod > 0) {
                QScopedPointer<KisPaintDevice::LodDataStruct> data(cachedProjection->createLodDataStruct(lod));
                cachedProjection->updateLodDataStruct(data.data(), extent);
                cachedProjection->uploadLodDataStruct(data.data());
            }

            m_d->updateCacheMetrics(source, compositor);
            m_d->cachedProjection = cachedProjection;
        }
    }

    return cachedProjection;
}

void KisOnionSkinCache::reset()
{
    QWriteLocker writeLocker(&m_d->lock);
    m_d->cachedProjection = 0;
}

KisPaintDeviceSP KisOnionSkinCache::lodCapableDevice() const
{
    return m_d->cachedProjection;
}
