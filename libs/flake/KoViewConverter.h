/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOVIEWCONVERTER_H
#define KOVIEWCONVERTER_H

#include <QPointF>
#include <QRectF>

/**
 * The interface for view conversions.
 * All KoShape based objects are using a postscript-point (pt) based measurement system
 * which requires a conversion to view coordinates (in pixel sizes) at the moment
 * we are painting, and a conversion to the normalized coordinate system if we
 * receive mouse events so we can figure out which KoShape object was touched.
 */
class KoViewConverter {
public:
    KoViewConverter() {};
    virtual ~KoViewConverter() {};

    /**
     * Convert a coordinate in pt to pixels.
     * @param documentPoint the point in the document coordinate system of a KoShape.
     */
    virtual QPointF documentToView( const QPointF &documentPoint ) const = 0;

    /**
     * Convert a coordinate in pixels to pt.
     * @param viewPoint the point in the coordinate system of the widget, or window.
     */
    virtual QPointF viewToDocument( const QPointF &viewPoint ) const = 0;

    /**
     * Convert a rectangle in pt to pixels.
     * @param documentRect the rect in the document coordinate system of a KoShape.
     */
    virtual QRectF documentToView( const QRectF &documentRect ) const = 0;

    /**
     * Convert a rectangle in pixels to pt.
     * @param viewRect the rect in the coordinate system of the widget, or window.
     */
    virtual QRectF viewToDocument( const QRectF &viewRect ) const = 0;

    /**
     * Convert a single x coordinate in pt to pixels.
     * @param documentX the x coordinate in pt.
     * @return the x coordinate in pixels.
     */
    virtual double documentToViewX( double documentX ) const = 0;

    /**
     * Convert a single y coordinate in pt to pixels.
     * @param documentY the y coordinate in pt.
     * @return the y coordinate in pixels.
     */
    virtual double documentToViewY( double documentY ) const = 0;

    /**
     * Convert a single x coordinate in pixels to pt.
     * @param viewX the x coordinate in pixels.
     * @return the x coordinate in pt.
     */
    virtual double viewToDocumentX( double viewX ) const = 0;

    /**
     * Convert a single y coordinate in pixels to pt.
     * @param viewY the y coordinate in pixels.
     * @return the y coordinate in pt.
     */
    virtual double viewToDocumentY( double viewY ) const = 0;

    /**
     * set the zoom levels of the individual x and y axis to the pointer paramets.
     * @param zoomX a pointer to a double which will be modified to set the horizontal zoom.
     * @param zoomY a pointer to a double which will be modified to set the vertical zoom.
     */
    virtual void zoom(double *zoomX, double *zoomY) const = 0;
};

#endif
