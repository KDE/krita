/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>
   Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>

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

#ifndef KOZOOMHANDLER_H
#define KOZOOMHANDLER_H

#include "kritawidgets_export.h"
#include <KoZoomMode.h>
#include <KoViewConverter.h>

/**
 * This class handles the zooming and DPI stuff (conversions between
 * postscript pt values and pixels). If the internal data of your
 * document does not work with postscript points (for instance raster
 * image pixels), you need to some additional converting yourself.
 *
 * An instance of KoZoomHandler operates at a given zoom  and resolution
 * so there is usually one instance of KoZoomHandler per view.
 */
class KRITAWIDGETS_EXPORT KoZoomHandler : public KoViewConverter
{
public:

    KoZoomHandler();
    ~KoZoomHandler() override;
    
    /**
     * @return the conversion factor between document and view, that
     * includes the zoom and also the DPI setting.
     */
    inline qreal zoomedResolutionX() const { return m_zoomedResolutionX; }

    /**
     * @return the conversion factor between document and view, that
     * includes the zoom and also the DPI setting.
     */
    inline qreal zoomedResolutionY() const { return m_zoomedResolutionY; }

    inline qreal resolutionX() const { return m_resolutionX; }
    inline qreal resolutionY() const { return m_resolutionY; }

    /**
     * Zoom factor for X. Equivalent to zoomedResolutionX()/resolutionX()
     */
    inline qreal zoomFactorX() const { return m_zoomedResolutionX / m_resolutionX; }

    /**
     * Zoom factor for Y. Equivalent to zoomedResolutionY()/resolutionY()
     */
    inline qreal zoomFactorY() const { return m_zoomedResolutionY / m_resolutionY; }


    /**
     * Set resolution expressed in dots-per-inch
     */
    void setDpi(int dpiX, int dpiY);

    /**
     * Set a resolution for X and Y of the output device.
     * The zoom factor is not changed.
     *
     * This number should be the result of:
     * POINT_TO_INCH(static_cast<qreal>(DOTS_PER_INCH))
     */
    void setResolution(qreal resolutionX, qreal resolutionY);

    /**
     * Set the resolution for X and Y to the display values reported by KGlobal.
     * The zoom factor is not changed.
     */
    void setResolutionToStandard( );

    /**
     * Set the zoomed resolution for X and Y.
     * Compared to the setZoom... methods, this allows to set a different
     * zoom factor for X and for Y.
     */
    virtual void setZoomedResolution(qreal zoomedResolutionX, qreal zoomedResolutionY);

    /**
     * Change the zoom level, keeping the resolution unchanged.
     * @param zoom the zoom factor (e.g. 1.0 for 100%)
     */
    void setZoom(qreal zoom) override;

    /**
     * Change the zoom mode
     * @param zoomMode the zoom mode.
     */
    inline void setZoomMode(KoZoomMode::Mode zoomMode) { m_zoomMode = zoomMode; }
    /**
     * @return the global zoom factor (e.g. 100 for 100%).
     * Only use this to display to the user, don't use in calculations
     */
    inline int zoomInPercent() const { return qRound(KoViewConverter::zoom() * 100); }
    /**
     * @return the global zoom mode (e.g. KoZoomMode::ZOOM_WIDTH).
     * use this to determine how to zoom
     */
    KoZoomMode::Mode zoomMode() const { return m_zoomMode; }

    // Input: pt. Output: pixels. Resolution and zoom are applied.

    inline qreal zoomItX(qreal z) const
        {
            return m_zoomedResolutionX * z;
        }

    inline qreal zoomItY(qreal z) const
        {
            return m_zoomedResolutionY * z ;
        }

    // Input: pixels. Output: pt.
    inline qreal unzoomItX(qreal x) const
        {
            return  x / m_zoomedResolutionX;
        }

    inline qreal unzoomItY(qreal y) const
        {
            return  y / m_zoomedResolutionY;
        }

    // KoViewConverter-interface methods

    /**
     * Convert a coordinate in pt to pixels.
     * @param documentPoint the point in the document coordinate system of a KoShape.
     */
    QPointF documentToView(const QPointF &documentPoint) const override;

    /**
     * Convert a coordinate in pixels to pt.
     * @param viewPoint the point in the coordinate system of the widget, or window.
     */
    QPointF viewToDocument(const QPointF &viewPoint) const override;

    /**
     * Convert a rectangle in pt to pixels.
     * @param documentRect the rect in the document coordinate system of a KoShape.
     */
    QRectF documentToView(const QRectF &documentRect) const override;

    /**
     * Convert a rectangle in pixels to pt.
     * @param viewRect the rect in the coordinate system of the widget, or window.
     */
    QRectF viewToDocument(const QRectF &viewRect) const override;

    /**
     * Convert a size in pt to pixels.
     * @param documentSize the size in pt.
     * @return the size in pixels.
     */
    QSizeF documentToView(const QSizeF &documentSize) const override;

    /**
     * Convert a size in pixels to pt.
     * @param viewSize the size in pixels.
     * @return the size in pt.
     */
    QSizeF viewToDocument(const QSizeF &viewSize) const override;

    /**
     * Convert a single x coordinate in pt to pixels.
     * @param documentX the x coordinate in pt.
     * @return the x coordinate in pixels.
     */
    qreal documentToViewX(qreal documentX) const override;

    /**
     * Convert a single y coordinate in pt to pixels.
     * @param documentY the y coordinate in pt.
     * @return the y coordinate in pixels.
     */
    qreal documentToViewY(qreal documentY) const override;

    /**
     * Convert a single x coordinate in pixels to pt.
     * @param viewX the x coordinate in pixels.
     * @return the x coordinate in pt.
     */
    qreal viewToDocumentX(qreal viewX) const override;

    /**
     * Convert a single y coordinate in pixels to pt.
     * @param viewY the y coordinate in pixels.
     * @return the y coordinate in pt.
     */
    qreal viewToDocumentY(qreal viewY) const override;

    /**
     * Get the zoom levels of the individual x and y axis. Copy them to the pointer parameters.
     * @param zoomX a pointer to a qreal which will be modified to set the horizontal zoom.
     * @param zoomY a pointer to a qreal which will be modified to set the vertical zoom.
     */
    void zoom(qreal *zoomX, qreal *zoomY) const override;

    using KoViewConverter::zoom;

protected:

    KoZoomMode::Mode m_zoomMode;

    qreal m_resolutionX;
    qreal m_resolutionY;
    qreal m_zoomedResolutionX;
    qreal m_zoomedResolutionY;
};

#endif
