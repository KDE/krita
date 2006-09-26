/*
 *  Copyright (c) 2006 Boudewijn Rempt
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

#ifndef KIS_ZOOM_HANDLER
#define KIS_ZOOM_HANDLER

#include <KoZoomHandler.h>


/**
 * Like KoTextZoomHandler, KisZoomHandler is a special zoomhandler
 * with additional support for raster images with a fictitious
 * resolution.
 *
 * KisZoomHandler handles the zoom conversions for Krita. Krita is
 * unique among KOffice applications that its internal coordinates are
 * not in postscript points, but in pixels.
 *
 * However, a Krita image also has a resolution, like 300 dpi, or rather,
 * 300 pixels-per-inch.
 *
 * Combined with the actual screen resolutions (say 100dpi) and the zoom
 * level, a Krita canvas will show the image at a particular scale.
 *
 * There is also a dot-for-dot mode, where one pixel of the image is one
 *pixel on screen.
*/
class KisZoomHandler : public KoZoomHandler {

Q_OBJECT

public:

    KisZoomHandler();
    virtual ~KisZoomHandler();


public:


    /**
     * Convert a coordinate in pt to pixels.
     * @param documentPoint the point in the document coordinate system of a KoShape.
     */
    virtual QPointF documentToView( const QPointF &documentPoint ) const;

    /**
     * Convert a coordinate in pixels to pt.
     * @param viewPoint the point in the coordinate system of the widget, or window.
     */
    virtual QPointF viewToDocument( const QPointF &viewPoint ) const;

    /**
     * Convert a rectangle in pt to pixels.
     * @param documentRect the rect in the document coordinate system of a KoShape.
     */
    virtual QRectF documentToView( const QRectF &documentRect ) const;

    /**
     * Convert a rectangle in pixels to pt.
     * @param viewRect the rect in the coordinate system of the widget, or window.
     */
    virtual QRectF viewToDocument( const QRectF &viewRect ) const;

    /**
     * Convert a single x coordinate in pt to pixels.
     * @param documentX the x coordinate in pt.
     * @return the x coordinate in pixels.
     */
    virtual double documentToViewX( double documentX ) const;

    /**
     * Convert a single y coordinate in pt to pixels.
     * @param documentY the y coordinate in pt.
     * @return the y coordinate in pixels.
     */
    virtual double documentToViewY( double documentY ) const;

    /**
     * Convert a single x coordinate in pixels to pt.
     * @param viewX the x coordinate in pixels.
     * @return the x coordinate in pt.
     */
    virtual double viewToDocumentX( double viewX ) const;

    /**
     * Convert a single y coordinate in pixels to pt.
     * @param viewY the y coordinate in pixels.
     * @return the y coordinate in pt.
     */
    virtual double viewToDocumentY( double viewY ) const;

    /**
     * set the zoom levels of the individual x and y axis to the pointer paramets.
     * @param zoomX a pointer to a double which will be modified to set the horizontal zoom.
     * @param zoomY a pointer to a double which will be modified to set the vertical zoom.
     */
    virtual void zoom(double *zoomX, double *zoomY) const;


};

#endif
