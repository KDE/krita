/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_view_converter.h"

#include <KoViewConverter.h>

KisViewConverter::KisViewConverter( double zoom, int documentDpi, int outputDpiX, int outputDpiY )
    : KoViewConverter()
    , m_zoom( zoom )
    , m_documentDpi( documentDpi )
    , m_outputDpiX( outputDpiX )
    , m_outputDpiY( outputDpiY )
{
}

void KisViewConverter::zoomTo( double zoom )
{
    m_zoom = zoom;
}

QPointF KisViewConverter::documentToView( const QPointF &documentPoint ) const
{
    return QPointF( documentPoint );
}

QPointF KisViewConverter::viewToDocument( const QPointF &viewPoint ) const
{
    return QPointF( viewPoint );
}

QRectF KisViewConverter::documentToView( const QRectF &documentRect ) const
{
    return QRectF ( documentRect );
}

QRectF KisViewConverter::viewToDocument( const QRectF &viewRect ) const
{
    return QRectF( viewRect );
}

double KisViewConverter::documentToViewX( double documentX ) const
{
    return documentX;
}

double KisViewConverter::documentToViewY( double documentY ) const
{
    return documentY;
}

double KisViewConverter::viewToDocumentX( double viewX ) const
{
    return viewX;
}

double KisViewConverter::viewToDocumentY( double viewY ) const
{
    return viewY;
}

void KisViewConverter::zoom(double *zoomX, double *zoomY) const
{
    Q_UNUSED(zoomX);
    Q_UNUSED(zoomY);    
}
