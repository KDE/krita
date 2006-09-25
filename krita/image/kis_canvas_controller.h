/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_CANVAS_CONTROLLER_H_
#define KIS_CANVAS_CONTROLLER_H_

#include <qglobal.h>
#include <QPoint>
#include <QRect>
#include "kis_types.h"

class QWidget;
class KisTool;
class KisRect;
class KisPoint;
class KisCanvas;
class KisInputDevice;

/**
 * Interface for classes that implement a canvas; i.e., a widget where KisImages
 * are painted onto. This is the "view" part of the model-view-controller paradigm;
 * the naming is a confusing historical artefact.
 */
class KisCanvasController {
public:
    KisCanvasController() {};
    virtual ~KisCanvasController() {};

public:

    /**
     * @return the canvas object
     */
    virtual KisCanvas *kiscanvas() const = 0;


    /**
     * @return the value of the horizontal scrollbar.
     */
    virtual qint32 horzValue() const = 0;

    /**
     * @return the value of the vertical scrollbar
     */
    virtual qint32 vertValue() const = 0;
    
    /**
     * Sets the horizontal and vertical scrollbars to the specified values
     *
     * @param x the value the horizontal scrollbar is set to
     * @param y the value the vertical scrollbar is set to
     */
    virtual void scrollTo(qint32 x, qint32 y) = 0;
    
    /**
     * Tell all of the canvas to repaint itself.
     */
    virtual void updateCanvas() = 0;


    /**
     * Tell the canvas to repaint the rectangle defined by x, y, w and h.
     * The coordinates are image coordinates.
     */
    virtual void updateCanvas(qint32 x, qint32 y, qint32 w, qint32 h) = 0;

    /**
     * Tell the canvas repaint the specified rectangle. The coordinates
     * are image coordinates, not view coordinates.
     */
    virtual void updateCanvas(const QRect& rc) = 0;
    
    /**
     * Increase the zoomlevel one step
     */
    virtual void zoomIn() = 0;

    /**
     * Increase the zoomlevel one step and make sure that x,y is the center point of the view.
     *
     * @param x The x coordinate of the visible point in image coordinates
     * @param y the y coordinate of the visible point in image coordinates
     */
    virtual void zoomIn(qint32 x, qint32 y) = 0;
    
    /**
     * Decrease the zoomlevel one step
     */
    virtual void zoomOut() = 0;


    /**
     * Decrease the zoomlevel one step and make sure that x,y is the center point of the view.
     *
     * @param x the x coordinate of the visible point in image coordinates
     * @param y the y coordinate of the visible point in image coordinates
     */
    virtual void zoomOut(qint32 x, qint32 y) = 0;
    
    /**
     * To center the view on the given point with the given zoom factor.
     *
     * @param x the x coordinate of the center point in image coordinates
     * @param y the y coordinate of the center point in image coordinates
     * @param zf the zoomfactor
     */
    virtual void zoomAroundPoint(double x, double y, double zf) = 0;

    /**
     * Make the rect defined by x, y, w and h visible, zooming in or
     * out as necessary. The view will be centered around the center point
     * of the specified rect.
     */
    virtual void zoomTo(qint32 x, qint32 y, qint32 w, qint32 h) = 0;

    /**
     * Make the rect defined by x, y, w and h visible, zooming in or
     * out as necessary. The view will be centered around the center point
     * of the specified rect.
     */
    virtual void zoomTo(const QRect& r) = 0;
    
    /**
     * Make the rect defined by x, y, w and h visible, zooming in or
     * out as necessary. The view will be centered around the center point
     * of the specified rect.
     */
    virtual void zoomTo(const KisRect& r) = 0;
    
    /**
     * Conversion functions from view coordinates to image coordinates
     *
     * You can get the rectangle of the image that's visible using the 
     * viewToWindow() functions (KisCanvasController). E.g. 
     * viewToWindow(QRect(0, 0, canvasWidth, canvasHeight)).
     *
     * Here, the view is the canvas widget in the view widget, and the window
     * is the window on the image.
     */
    virtual QPoint viewToWindow(const QPoint& pt) = 0;
    virtual KisPoint viewToWindow(const KisPoint& pt) = 0;
    virtual QRect viewToWindow(const QRect& rc) = 0;
    virtual KisRect viewToWindow(const KisRect& rc) = 0;
    virtual void viewToWindow(qint32 *x, qint32 *y) = 0;
    
    /**
     * Conversion functions from image coordinates to view coordinates
     */
    virtual QPoint windowToView(const QPoint& pt) = 0;
    virtual KisPoint windowToView(const KisPoint& pt) = 0;
    virtual QRect windowToView(const QRect& rc) = 0;
    virtual KisRect windowToView(const KisRect& rc) = 0;
    virtual void windowToView(qint32 *x, qint32 *y) = 0;
    
    /**
     * Set the cursor shown when the pointer is over the canvas widget to 
     * the specified cursor.
     *
     * @param cursor the new cursor
     * @return the old cursor
     */
    virtual QCursor setCanvasCursor(const QCursor & cursor) = 0;
    
    /**
     * @return the current input device, such as a mouse or a stylus
     */
    virtual KisInputDevice currentInputDevice() const = 0;


private:
    KisCanvasController(const KisCanvasController&);
    KisCanvasController& operator=(const KisCanvasController&);
};

#endif // KIS_CANVAS_CONTROLLER_H_

