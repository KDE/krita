/*
 *  Copyright (c) 2007, Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008, Cyrille Berger <cberger@cberger.net>
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
#include <KoColorProfile.h>

#include "kis_projection_cache.h"
#include "kis_config.h"
#include <kis_image.h>

KisProjectionCache::KisProjectionCache()
        : m_cacheKisImageAsQImage(true)
        , m_monitorProfile(0)
{
}

void KisProjectionCache::setImage(KisImageWSP image)
{
    m_image = image;
}

// This is a workaround against border effects
// and indirect painting support
#define ALWAYS_CACHE_AS_QIMAGE

void KisProjectionCache::setCacheKisImageAsQImage(bool toggle)
{
#ifdef ALWAYS_CACHE_AS_QIMAGE
    toggle = true;
#endif
    if (toggle != m_cacheKisImageAsQImage) {
        if (toggle) {
            m_cacheKisImageAsQImage = true;
            updateCachedQImage(QRect(0, 0, m_imageSize.width(), m_imageSize.height()));
        } else {
            m_cacheKisImageAsQImage = false;
            m_unscaledCache = QImage();
        }
    }
}

void KisProjectionCache::setImageSize(qint32 w,  qint32 h)
{
    dbgRender << "KisProjectionCache::setImageSize Setting image size from " << m_imageSize << " to " << w << ", " << h;


    m_imageSize = QSize(w, h);

    if (!m_cacheKisImageAsQImage) return;

#ifndef ALWAYS_CACHE_AS_QIMAGE
    KisConfig cfg;
    quint32 maxCachedImageSize = cfg.maxCachedImageSize();

    if (((w * h) / 1000000) > maxCachedImageSize) {
        setCacheKisImageAsQImage(false);
    } else {
#endif

        m_unscaledCache = QImage(w, h, QImage::Format_ARGB32);
        /**
         * FIXME: Check why not use m_image->bounds()
         * instead of QRect(0, 0, w, h)
         */
        if (m_cacheKisImageAsQImage)
            updateCachedQImage(QRect(0, 0, w, h));

#ifndef ALWAYS_CACHE_AS_QIMAGE
    }
#endif
}

void KisProjectionCache::setMonitorProfile(const KoColorProfile* monitorProfile)
{
    m_monitorProfile = monitorProfile;
}

void KisProjectionCache::setDirty(UpdateInformation &info)
{
    if (!m_image) return;
    if (m_cacheKisImageAsQImage) {
        updateCachedQImage(info.dirtyImageRect);
    }
    else {
        /**
         * We can neither draw the image directly from the KisImage
         * nor cache it in the internal QImage.
         * The only way out left is to create a patch and store
         * a dirty piece of image there
         */
        /* if(info.transfer == UpdateInformation::DIRECT) */
        info.transfer = UpdateInformation::PATCH;
    }

    if (info.transfer == UpdateInformation::PATCH)
        info.patch = getNearestPatch(info);
}

void KisProjectionCache::updateCachedQImage(const QRect &rect)
{
    if (m_unscaledCache.isNull())
        m_unscaledCache = QImage(m_image->width(), m_image->height(), QImage::Format_ARGB32);

    QPainter gc(&m_unscaledCache);
    gc.setCompositionMode(QPainter::CompositionMode_Source);

    QImage updateImage = m_image->convertToQImage(rect,
                                                  m_monitorProfile);

    gc.drawImage(rect.x(), rect.y(),
                 updateImage, 0, 0,
                 rect.width(), rect.height());
    gc.end();
}

KisImagePatch KisProjectionCache::getNearestPatch(UpdateInformation &info)
{
    KisImagePatch patch;

    patch.m_scaleX = 1.;
    patch.m_scaleY = 1.;

    patch.m_interestRect = QRectF(info.borderWidth, info.borderWidth,
                                  info.imageRect.width(),
                                  info.imageRect.height());

    QRect adjustedRect = info.imageRect.adjusted(-info.borderWidth, -info.borderWidth,
                                                 info.borderWidth, info.borderWidth);
    patch.m_patchRect = adjustedRect;

    if (m_cacheKisImageAsQImage)
        patch.m_image = m_unscaledCache.copy(patch.m_patchRect);
    else
        patch.m_image = m_image->convertToQImage(patch.m_patchRect,
                                                 m_monitorProfile);

    return patch;
}

void KisProjectionCache::drawFromOriginalImage(QPainter& gc, UpdateInformation &info)
{
    if (m_cacheKisImageAsQImage) {
        gc.save();
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.setRenderHints(info.renderHints, true);
        gc.drawImage(info.viewportRect, m_unscaledCache, info.imageRect);
        gc.restore();
    } else {
        Q_ASSERT(info.patch.isValid());
        info.patch.drawMe(gc, info.viewportRect, info.renderHints);
    }
}
