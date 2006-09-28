/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCANVASBASE_H
#define KOCANVASBASE_H

#include <KoUnit.h>

#include <QPointF>
#include <QPoint>
#include <QRectF>
#include <QRect>
#include <QWidget>

class KoSelection;
class KCommand;
class KoShapeManager;
class KoTool;
class KoViewConverter;

/**
 * KoCanvasBase is the interface actual application canvas classes
 * should implement. Flake tools know about the canvas, so they can
 * do things like scroll, redraw, set a cursor etc.
 */
class KoCanvasBase {
public:
    KoCanvasBase() {};
    virtual ~KoCanvasBase() {};

public:

    /**
     * retrieve the grid size setting.
     * The grid spacing will be provided in pt.
     * @param horizontal a pointer to a double that will be filled with the horizontal grid-spacing
     * @param vertical a pointer to a double that will be filled with the vertical grid-spacing
     */
    virtual void gridSize(double *horizontal, double *vertical) const = 0;

    /**
     * return if snap to grid is enabled.
     * @return if snap to grid is enabled.
     */
    virtual bool snapToGrid() const = 0;

    /**
     * Adds a command to the history. Call this for each @p command you create.
     * Unless you set @p execute to false, this will also execute the command.
     * This means, most of the application's code will look like
     *    MyCommand * cmd = new MyCommand( parameters );
     *    canvas.addCommand( cmd );
     *
     * Note that the command history takes ownership of the command, it will delete
     * it when the undo limit is reached, or when deleting the command history itself.
     * @param command the command to add
     * @param execute if true, the commands execute method will be called
     */
    virtual void addCommand(KCommand *command, bool execute = true) = 0;

    /**
     * return the current shapeManager
     * @return the current shapeManager
     */
    virtual KoShapeManager *shapeManager() const = 0;

    /**
     * Tell the canvas repaint the specified rectangle. The coordinates
     * are document coordinates, not view coordinates.
     */
    virtual void updateCanvas(const QRectF& rc) = 0;

    /**
     * Return the curently active tool, or 0 if non active.
     * @return the curently active tool, or 0 if non active.
     */
    virtual KoTool* tool() = 0;

    /**
     * Set the new ative tool.
     * @param tool the new tool to be used on the canvas.
     */
    virtual void setTool(KoTool *tool) = 0;

    /**
     * Return the viewConverter for this view.
     * @return the viewConverter for this view.
     */
    virtual KoViewConverter *viewConverter() = 0;

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual QWidget* canvasWidget() = 0;

    /**
     * Return the unit of the current document for initialization of the widgets created
     * by the flake framework.
     * @see KoDocument::unit()
     */
    virtual KoUnit::Unit unit() = 0;

#if 0
/*  The next list of methods are naming taken from Krita, which means they have already been
    toughened by time.  So, if you ever need a method in this interface; please uncomment the
    respective one here for good naming conventions.  It will probably save you time in
    implementing some of the methods in the different KOffice apps as well ;)
*/


    virtual KoSelection * currentSelection() const 0;

    /**
     * @return the value of the horizontal scrollbar.
     */
    virtual qint32 horizontalScrollbarValue() const  0;

    /**
     * @return the value of the vertical scrollbar
     */
    virtual qint32 verticalScrollbarValue() const  0;

    /**
     * Sets the horizontal and vertical scrollbars to the specified values
     *
     * @param x the value the horizontal scrollbar is set to
     * @param y the value the vertical scrollbar is set to
     */
    virtual void scrollTo(qint32 x, qint32 y)  0;

    /**
     * Tell all of the canvas to repaint itself.
     */
    virtual void updateCanvas()  0;

    /**
     * Tell the canvas to repaint the rectangle defined by x, y, w and h.
     * The coordinates are document coordinates.
     */
    virtual void updateCanvas(qint32 x, qint32 y, qint32 w, qint32 h)  0;

    /**
     * Increase the zoomlevel one step
     */
    virtual void zoomIn()  0;

    /**
      Increase the zoomlevel one step and make sure that x,y is the center
       point of the view.

      @param x The x coordinate of the visible point in document
               coordinates
      @param y the y coordinate of the visible point in document
               coordinates
     */
    virtual void zoomIn(qint32 x, qint32 y)  0;

    /**
     * Decrease the zoomlevel one step
     */
    virtual void zoomOut()  0;


    /**
     * Decrease the zoomlevel one step and make sure that x,y is the center point of the view.
     *
     * @param x the x coordinate of the visible point in document coordinates
     * @param y the y coordinate of the visible point in document coordinates
     */
    virtual void zoomOut(qint32 x, qint32 y)  0;

    /**
     * To center the view on the given point with the given zoom factor.
     *
     * @param x the x coordinate of the center point in document coordinates
     * @param y the y coordinate of the center point in document coordinates
     * @param zf the zoomfactor
     */
    virtual void zoomAroundPoint(double x, double y, double zf)  0;

    /**
     * Make the rect defined by x, y, w and h visible, zooming in or
     * out as necessary. The view will be centered around the center point
     * of the specified rect.
     */
    virtual void zoomTo(qint32 x, qint32 y, qint32 w, qint32 h)  0;

    /**
     * Make the rect defined by x, y, w and h visible, zooming in or
     * out as necessary. The view will be centered around the center point
     * of the specified rect.
     */
    virtual void zoomTo(const QRect& r)  0;

    /**
     * Make the rect defined by x, y, w and h visible, zooming in or
     * out as necessary. The view will be centered around the center point
     * of the specified rect.
     */
    virtual void zoomTo(const QRectF& r)  0;

    /**
     * Conversion functions from view coordinates to document coordinates
     *
     * You can get the rectangle of the document that's visible using the
     * viewToWindow() functions (KoCanvasBase). E.g.
     * viewToWindow(QRect(0, 0, canvasWidth, canvasHeight)).
     *
     * Here, the view is the canvas widget in the view widget, and the window
     * is the window on the document.
     */
    virtual QPoint viewToWindow(const QPoint& pt)  0;
    virtual QPointF viewToWindow(const QPointF& pt)  0;
    virtual QRect viewToWindow(const QRect& rc)  0;
    virtual QRectF viewToWindow(const QRectF& rc)  0;
    virtual void viewToWindow(qint32 *x, qint32 *y)  0;

    /**
     * Conversion functions from document coordinates to view coordinates
     */
    virtual QPoint windowToView(const QPoint& pt)  0;
    virtual QPointF windowToView(const QPointF& pt)  0;
    virtual QRect windowToView(const QRect& rc)  0;
    virtual QRectF windowToView(const QRectF& rc)  0;
    virtual void windowToView(qint32 *x, qint32 *y)  0;

    /**
     * Set the cursor shown when the pointer is over the canvas widget to
     * the specified cursor.
     *
     * @param cursor the new cursor
     * @return the old cursor
     */
    virtual QCursor setCanvasCursor(const QCursor & cursor)  0;

    /**
     * Set the active input device to the specified input device, This
     * could be a mouse, a stylus, an eraser or any other pointing input
     * device.
     *
     * @param inputDevice the new input device
     */
    virtual void setInputDevice(KoInputDevice inputDevice)  0;

    /**
     * @return the current input device, such as a mouse or a stylus
     */
    virtual KoInputDevice currentInputDevice() const  0;
#endif
};

#endif
