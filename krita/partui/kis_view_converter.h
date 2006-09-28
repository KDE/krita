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

#ifndef KIS_VIEW_CONVERTER
#define KIS_VIEW_CONVERTER

#include <KoViewConverter.h>


/**
 * KisViewConverter handles all calculations between the pixels on the screen and
 * the pixels in the document. Krita does not use postscript points internally, but
 * pixels. It is a raster image editor, after all. However, I have been told that
 * users of raster images believe that a raster image can have something they call
 * "resolution". That is, the fiction that their image is x inches by y inches.
 *
 * So... We have to compare the screen dpi (which may, and will often be, wrong,
 * because Windows, OS X and many brain-dread Linux distributions hide the real
 * screen resolution and use a fictitious 96 dpi instead) with the fictitious
 * image resolution to show the image scaled to the right amount of inches at 100%
 * zoom level.
 *
 * For instance, if the image dpi is 1000 and the screen dpi is 100x100, then we will
 * show 1 pixel on screen for every 10 x 10 pixels of the image on screen. On a 500 dpi
 * printer, we would use 2 x 2 image pixels for 1 printer dot.
 *
 * If the image dpi is 10, and the screen dpi is still 100 x 100, then we will blow up
 * every pixel to cover 10 x 10 pixels on the screen.
 * 
 * Note:
 *
 * If a user changes the resolution of the image, the size in inches will change. This
 * will not scale the image to consist of more or less actual pixels.
 *
 * Note also: 
 *
 * We may have clashes with the point-based flake system. There are 72 Flake points to
 * an inch, we may need to take that into account in the document-to-view calculations.
 *
 * Don't forget:
 *
 * Screens commonly have non-square pixels, image documents (as far as I can tell) never.
 * 
 */
class KisViewConverter : public KoViewConverter {

public:

    /**
     * Create a new KisViewConverter. There is should be one view converter per
     * view, and maybe one more for printing
     * 
     * @param zoom the initial zoomlevel of this view converter 1.0 == 100%
     * @param documentDpi the initial resolution of the document in pixels/inch
     * @param outputDpiX the horizontal resolution of the output device
     * @param outputDpiY the vertical resolution of the output device
     */
    KisViewConverter(double zoom, int documentDpi, int outputDpiX, int outputDpiY);
    virtual ~KisViewConverter() {};
    
    void setDocumentDpi(int documentDpi) { m_documentDpi = documentDpi; }
    int documentDpi() { return m_documentDpi; }

    void setOutputDpiX(int outputDpiX) { m_outputDpiX = outputDpiX; }
    int outputDpiX() { return m_outputDpiX; }

    void setOutputDpiY(int outputDpiY) { m_outputDpiY = outputDpiY; }
    int outputDpiY() { return m_outputDpiY; }

    /**
     * Set the zoom level of this view converter to zoom
     *
     * @param zoom the new zoom level 1.0 = 100%
     */
    void zoomTo(double zoom);
    double zoomLevel() { return m_zoom; }

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
     * set the zoom levels of the individual x and y axis to the
     * pointer parameters.
     *
     * @param zoomX a pointer to a double which will be modified to set the horizontal zoom.
     * @param zoomY a pointer to a double which will be modified to set the vertical zoom.
     */
    virtual void zoom(double *zoomX, double *zoomY) const;

private:

    double m_zoom;
    int m_documentDpi;
    int m_outputDpiX;
    int m_outputDpiY;
};

#endif
