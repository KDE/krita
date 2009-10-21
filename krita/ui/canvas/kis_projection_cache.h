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

#ifndef KIS_PROJECTION_CACHE
#define KIS_PROJECTION_CACHE

#include <QImage>
#include <QSize>
#include <QRect>
#include <kis_types.h>

#include "kis_projection_backend.h"

/**
 * The projection cache papers over the option
 * to cache the image projection as one big qimage,
 * to not cache it as a qimage at all, or in any
 * other way we have not invented yet.
 */
class KisProjectionCache : public KisProjectionBackend
{
public:
    KisProjectionCache();
    virtual ~KisProjectionCache() {}

    void setImage(KisImageWSP image);
    void setImageSize(qint32 w, qint32 h);
    void setMonitorProfile(const KoColorProfile* monitorProfile);
    void setDirty(const QRect& rc);

    KisImagePatch getNearestPatch(qreal scaleX, qreal scaleY,
                                  const QRect& requestedRect,
                                  qint32 borderWidth);

    void drawFromOriginalImage(QPainter& gc,
                               const QRect& imageRect,
                               const QRectF& viewportRect,
                               qint32 borderWidth,
                               QPainter::RenderHints renderHints);

    void setCacheKisImageAsQImage(bool toggle);


private:

    bool m_cacheKisImageAsQImage;

    QImage m_unscaledCache;
    KisImageWSP m_image;
    QSize m_imageSize;
    const KoColorProfile * m_monitorProfile;
};


#endif
