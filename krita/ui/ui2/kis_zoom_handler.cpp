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

#include <kis_zoom_handler.h>


KisZoomHandler::KisZoomHandler()
    : KoZoomHandler()
{
}

KisZoomHandler::~KisZoomHandler()
{
}


QPointF documentToView( const QPointF &documentPoint ) const
{
    return QPointF( documentPoint );
}

QPointF viewToDocument( const QPointF &viewPoint ) const
{
}

QRectF documentToView( const QRectF &documentRect ) const
{
}

QRectF viewToDocument( const QRectF &viewRect ) const
{
}

double documentToViewX( double documentX ) const
{
}

double documentToViewY( double documentY ) const
{
}

double viewToDocumentX( double viewX ) const
{
}

double viewToDocumentY( double viewY ) const
{
}

void zoom(double *zoomX, double *zoomY) const
{
}
