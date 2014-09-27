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

#ifndef KOPAUTIL_H
#define KOPAUTIL_H

#include "kopageapp_export.h"

class QSize;
class QRect;
struct KoPageLayout;
class KoZoomHandler;


class KOPAGEAPP_EXPORT KoPAUtil
{
public:
    /**
     * Set the zoom so the page fully fits into size
     */
    static void setZoom( const KoPageLayout & pageLayout, const QSize & size, KoZoomHandler & zoomHandler );

    /**
     * Set the zoom and adapt the size, so the page fully fits into size and the size matches the page rect
     */
    static void setSizeAndZoom(const KoPageLayout &pageLayout, QSize &size, KoZoomHandler &zoomHandler);

    /**
     * Get the page rect used
     */
    static QRect pageRect( const KoPageLayout & pageLayout, const QSize & size, const KoZoomHandler & zoomHandler );
};

#endif /* KOPAUTIL_H */
