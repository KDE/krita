/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoZoomHandler.h"
#include <kdebug.h>
#include <KoUnit.h> // for POINT_TO_INCH
#include <KoGlobal.h>

KoZoomHandler::KoZoomHandler()
{
    // Note that this calls the method below, not the derived one
    setZoomAndResolution( 100, KoGlobal::dpiX(), KoGlobal::dpiY() );
}

void KoZoomHandler::setZoomAndResolution( int zoom, int dpiX, int dpiY )
{
    // m_resolution[XY] is in pixel per pt
    m_resolutionX = POINT_TO_INCH( static_cast<double>(dpiX) );
    m_resolutionY = POINT_TO_INCH( static_cast<double>(dpiY) );
    setZoom( zoom );
    /*kDebug(32500) << "KoZoomHandler::setZoomAndResolution " << zoom << " " << dpiX << "," << dpiY
              << " m_resolutionX=" << m_resolutionX
              << " m_zoomedResolutionX=" << m_zoomedResolutionX
              << " m_resolutionY=" << m_resolutionY
              << " m_zoomedResolutionY=" << m_zoomedResolutionY << endl;*/
}

void KoZoomHandler::setResolution( double resolutionX, double resolutionY )
{
    m_zoom = 100;
    m_resolutionX = resolutionX;
    m_resolutionY = resolutionY;
    m_zoomedResolutionX = resolutionX;
    m_zoomedResolutionY = resolutionY;
}

void KoZoomHandler::setZoomedResolution( double zoomedResolutionX, double zoomedResolutionY )
{
    // m_zoom doesn't matter, it's only used in setZoom() to calculated the zoomed resolutions
    // Here we know them. The whole point of this method is to allow a different zoom factor
    // for X and for Y, as can be useful for e.g. fullscreen kpresenter presentations.
    m_zoomedResolutionX = zoomedResolutionX;
    m_zoomedResolutionY = zoomedResolutionY;
}

void KoZoomHandler::setZoom( int zoom )
{
    m_zoom = zoom;
    if( m_zoom == 100 ) {
        m_zoomedResolutionX = m_resolutionX;
        m_zoomedResolutionY = m_resolutionY;
    } else {
        m_zoomedResolutionX = static_cast<double>(m_zoom) * m_resolutionX / 100.0;
        m_zoomedResolutionY = static_cast<double>(m_zoom) * m_resolutionY / 100.0;
    }
}

QPointF KoZoomHandler::normalToView( const QPointF &normalPoint ) {
    return QPointF( zoomItX( normalPoint.x() ), zoomItY( normalPoint.y() ) );
}

QPointF KoZoomHandler::viewToNormal( const QPointF &viewPoint ) {
    return QPointF( unzoomItX( viewPoint.x() ), unzoomItY( viewPoint.y() ) );
}

QRectF KoZoomHandler::normalToView( const QRectF &normalRect ) {
    QRectF r;
    r.setCoords( zoomItX( normalRect.left() ),  zoomItY( normalRect.top() ),
                  zoomItX( normalRect.right() ), zoomItY( normalRect.bottom() ) );
    return r;
}

QRectF KoZoomHandler::viewToNormal( const QRectF &viewRect ) {
    QRectF r;
    r.setCoords( unzoomItX( viewRect.left() ),  unzoomItY( viewRect.top() ),
                  unzoomItX( viewRect.right() ), unzoomItY( viewRect.bottom() ) );
    return r;
}

void KoZoomHandler::zoom(double *zoomX, double *zoomY) const {
    *zoomX = zoomItX(100.0) / 100.0;
    *zoomY = zoomItY(100.0) / 100.0;
}
