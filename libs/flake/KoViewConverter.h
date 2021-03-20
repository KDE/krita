/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOVIEWCONVERTER_H
#define KOVIEWCONVERTER_H

#include "kritaflake_export.h"

#include <QtGlobal>

class QPointF;
class QRectF;
class QSizeF;
class QTransform;

/**
 * The interface for view conversions.
 *
 * All KoShape based objects are using a postscript-point (pt) based measurement system
 * which requires a conversion to view coordinates (in pixel sizes) at the moment
 * we are painting, and a conversion to the normalized coordinate system if we
 * receive mouse events so we can figure out which KoShape object was touched.
 *
 * The zoom level is expressed on a scale of 0.0 to 1.0 to infinite, where 1.0 is
 * 100%
 */
class KRITAFLAKE_EXPORT KoViewConverter
{
public:
    KoViewConverter();
    virtual ~KoViewConverter() {}

    /**
     * Convert a coordinate in pt to pixels.
     * @param documentPoint the point in the document coordinate system of a KoShape.
     */
    virtual QPointF documentToView(const QPointF &documentPoint) const;

    /**
     * Convert a coordinate in pixels to pt.
     * @param viewPoint the point in the coordinate system of the widget, or window.
     */
    virtual QPointF viewToDocument(const QPointF &viewPoint) const;

    /**
     * Convert a rectangle in pt to pixels.
     * @param documentRect the rect in the document coordinate system of a KoShape.
     */
    virtual QRectF documentToView(const QRectF &documentRect) const;

    /**
     * Convert a rectangle in pixels to pt.
     * @param viewRect the rect in the coordinate system of the widget, or window.
     */
    virtual QRectF viewToDocument(const QRectF &viewRect) const;

    /**
     * Convert a size in pt to pixels.
     * @param documentSize the size in pt.
     * @return the size in pixels.
     */
    virtual QSizeF documentToView(const QSizeF& documentSize) const;

    /**
     * Convert a size in pixels to pt.
     * @param viewSize the size in pixels.
     * @return the size in pt.
     */
    virtual QSizeF viewToDocument(const QSizeF& viewSize) const;

    /**
     * Convert a single x coordinate in pt to pixels.
     * @param documentX the x coordinate in pt.
     * @return the x coordinate in pixels.
     */
    virtual qreal documentToViewX(qreal documentX) const;

    /**
     * Convert a single y coordinate in pt to pixels.
     * @param documentY the y coordinate in pt.
     * @return the y coordinate in pixels.
     */
    virtual qreal documentToViewY(qreal documentY) const;

    /**
     * Convert a single x coordinate in pixels to pt.
     * @param viewX the x coordinate in pixels.
     * @return the x coordinate in pt.
     */
    virtual qreal viewToDocumentX(qreal viewX) const;

    /**
     * Convert a single y coordinate in pixels to pt.
     * @param viewY the y coordinate in pixels.
     * @return the y coordinate in pt.
     */
    virtual qreal viewToDocumentY(qreal viewY) const;

    /**
     * Retrieve the zoom levels of the individual x and y axes.
     * @param zoomX a pointer to a qreal which will be modified to the horizontal zoom.
     * @param zoomY a pointer to a qreal which will be modified to the vertical zoom.
     */
    virtual void zoom(qreal *zoomX, qreal *zoomY) const;

    /**
     * Set the zoom level. 1.0 is 100%.
     */
    virtual void setZoom(qreal zoom);

    /**
     * Return the current zoom level. 1.0 is 100%.
     */
    qreal zoom() const;

    QTransform documentToView() const;
    QTransform viewToDocument() const;

    virtual QTransform viewToWidget() const;
    virtual QTransform widgetToView() const;

private:
    qreal m_zoomLevel; // 1.0 is 100%
};

#endif
