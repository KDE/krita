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

class KoColorProfile;


/**
 * The projection cache papers over the option
 * to cache the image projection as one big qimage,
 * to not cache it as a qimage at all, or in any
 * other way we have not invented yet.
 */
class KisProjectionCache {

public:

    KisProjectionCache();

    void setImage( KisImageSP image );

    void setImageSize( qint32 w, qint32 h );

    void setCacheKisImageAsQImage( bool toggle );

    void setMonitorProfile( const KoColorProfile* monitorProfile );

    QImage image( const QRect& alignedImageRect );

    void drawImage( QPainter& gc, const QPointF& topleftUnscaled, const QRect& alignedImageRect );

    void updateUnscaledCache(const QRect& rc);

private:

    bool m_cacheKisImageAsQImage;

    QImage m_unscaledCache;
    KisImageSP m_image;
    QSize m_imageSize;
    const KoColorProfile * m_monitorProfile;
};


#endif
