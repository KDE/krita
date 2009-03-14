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
    : m_cacheKisImageAsQImage( true )
    , m_monitorProfile( 0 )
{
}

void KisProjectionCache::setImage( KisImageSP image )
{
    m_image = image;
}

void KisProjectionCache::setCacheKisImageAsQImage( bool toggle )
{
    if ( toggle != m_cacheKisImageAsQImage ) {
        if ( toggle ) {
            m_cacheKisImageAsQImage = true;
            updateUnscaledCache( QRect( 0, 0, m_imageSize.width(), m_imageSize.height() ) );
        }
        else {
            m_cacheKisImageAsQImage = false;
            m_unscaledCache = QImage();
        }
    }
}

void KisProjectionCache::setImageSize( qint32 w,  qint32 h )
{
    dbgRender << "KisProjectionCache::setImageSize Setting image size from " << m_imageSize << " to " << w << ", " << h;


    m_imageSize = QSize(w, h);

    if ( !m_cacheKisImageAsQImage ) return;

    KisConfig cfg;
    quint32 maxCachedImageSize = cfg.maxCachedImageSize();

    if ((( w * h ) / 1000000 ) > maxCachedImageSize ) {
        setCacheKisImageAsQImage( false );
    }
    else {
        m_unscaledCache = QImage(w, h, QImage::Format_ARGB32);
        updateUnscaledCache(QRect(0, 0, w, h));
    }
}

void KisProjectionCache::setMonitorProfile( const KoColorProfile* monitorProfile )
{
    m_monitorProfile = monitorProfile;
}

void KisProjectionCache::updateUnscaledCache(const QRect & rc)
{


    if ( !m_image ) return;
    if ( !m_cacheKisImageAsQImage ) return;

    if (m_unscaledCache.isNull()) {

        m_unscaledCache = QImage(m_image->width(), m_image->height(), QImage::Format_ARGB32);

    }

    QPainter p(&m_unscaledCache);
    p.setCompositionMode(QPainter::CompositionMode_Source);

    QImage updateImage = m_image->convertToQImage(rc.x(), rc.y(), rc.width(), rc.height(),
                                                  m_monitorProfile);

//     if (m_showMask && m_drawMaskVisualisationOnUnscaledCanvasCache) {
//
//         // XXX: Also visualize the global selection
//
//         KisSelectionSP selection = 0;
//
//         if (m_currentNode) {
//             if (m_currentNode->inherits("KisMask")) {
//
//                 selection = dynamic_cast<const KisMask*>(m_currentNode.data())->selection();
//             } else if (m_currentNode->inherits("KisLayer")) {
//
//                 KisLayerSP layer = dynamic_cast<KisLayer*>(m_currentNode.data());
//                 if (KisSelectionMaskSP selectionMask = layer->selectionMask()) {
//                     selection = selectionMask->selection();
//                 }
//             }
//         }
//         selection->paint(&updateImage, rc);
//     }


    p.drawImage(rc.x(), rc.y(), updateImage, 0, 0, rc.width(), rc.height());
    p.end();

}

QImage KisProjectionCache::image( const QRect& alignedImageRect )
{

    if ( m_cacheKisImageAsQImage ) {
        return m_unscaledCache.copy( alignedImageRect );
    }
    else {
        return m_image->convertToQImage( alignedImageRect.x(), alignedImageRect.y(),
                                         alignedImageRect.width(), alignedImageRect.height(),
                                         m_monitorProfile );
    }
}

void KisProjectionCache::drawImage( QPainter& gc, const QPointF& topLeftUnscaled, const QRect& alignedImageRect )
{
    if ( m_cacheKisImageAsQImage ) {
        gc.drawImage( topLeftUnscaled, m_unscaledCache, alignedImageRect );
    }
    else {
        QImage img = image( alignedImageRect );
        gc.drawImage( topLeftUnscaled, img );
    }
}
