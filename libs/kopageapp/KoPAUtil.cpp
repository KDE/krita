/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPAUtil.h"

#include <QRect>
#include <QSize>

#include <KoPageLayout.h>
#include <KoZoomHandler.h>

void KoPAUtil::setZoom( const KoPageLayout & pageLayout, const QSize & size, KoZoomHandler & zoomHandler )
{
    qreal zoom = size.width() / ( zoomHandler.resolutionX() * pageLayout.width );
    zoom = qMin( zoom, size.height() / ( zoomHandler.resolutionY() * pageLayout.height ) );
    zoomHandler.setZoom( zoom );
}

void KoPAUtil::setSizeAndZoom(const KoPageLayout &pageLayout, QSize &thumbnailSize, KoZoomHandler &zoomHandler)
{
    const qreal realWidth = zoomHandler.resolutionX() * pageLayout.width;
    const qreal realHeight = zoomHandler.resolutionY() * pageLayout.height;

    const qreal widthScale = thumbnailSize.width() / realWidth;
    const qreal heightScale = thumbnailSize.height() / realHeight;

    // adapt thumbnailSize to match the rendered page
    if (widthScale > heightScale) {
        const int thumbnailWidth = qMin(thumbnailSize.width(), qRound(realWidth*heightScale));
        thumbnailSize.setWidth(thumbnailWidth);
    } else {
        const int thumbnailHeight = qMin(thumbnailSize.height(), qRound(realHeight*widthScale));
        thumbnailSize.setHeight(thumbnailHeight);
    }

    // set zoom
    const qreal zoom = (widthScale > heightScale) ? heightScale : widthScale;
    zoomHandler.setZoom(zoom);
}

QRect KoPAUtil::pageRect( const KoPageLayout & pageLayout, const QSize & size, const KoZoomHandler & zoomHandler )
{
    int width = int( 0.5 + zoomHandler.documentToViewX( pageLayout.width ) );
    int height = int( 0.5 + zoomHandler.documentToViewY( pageLayout.height ) );
    int x = int( ( size.width() - width ) / 2.0 );
    int y = int( ( size.height() - height ) / 2.0 );
    return QRect( x, y, width, height );
}
