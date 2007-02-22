/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOCANVASBASE_H
#define KOCANVASBASE_H

#include <KoUnit.h>

#include <QPointF>
#include <QPoint>
#include <QRectF>
#include <QRect>
#include <QRegion>
#include <QWidget>

#include "flake_export.h"

class KoCanvasResourceProvider;
class KoSelection;
class QUndoCommand;
class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class KoShapeController;
class KoShapeControllerBase;

/**
 * KoCanvasBase is the interface actual application canvas classes
 * should implement. Flake tools know about the canvas, so they can
 * do things like scroll, redraw, set a cursor etc.
 */
class FLAKE_EXPORT KoCanvasBase {

public:

    /**
     * The constructor.
     * @param shapeControllerBase the implementation of the shapeController that the
     *   application provides to allow shapes to be added in multiple views.
     */
    explicit KoCanvasBase( KoShapeControllerBase * shapeControllerBase );
    virtual ~KoCanvasBase();

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
     * This will also execute the command.
     * This means, most of the application's code will look like
     *    MyCommand * cmd = new MyCommand( parameters );
     *    canvas.addCommand( cmd );
     *
     * Note that the command history takes ownership of the command, it will delete
     * it when the undo limit is reached, or when deleting the command history itself.
     * @param command the command to add
     */
    virtual void addCommand(QUndoCommand *command) = 0;

    /**
     * return the current shapeManager
     * @return the current shapeManager
     */
    virtual KoShapeManager *shapeManager() const = 0;

    /**
     * Tell the canvas to repaint the specified rectangle. The coordinates
     * are document coordinates, not view coordinates.
     */
    virtual void updateCanvas(const QRectF& rc) = 0;

    /**
     * This method should not be here; a QRegion is integer based and thus can only be used on whole points.
     * Code should use the QRectF version instead.
     */
    virtual void updateCanvas(const QRegion & region) KDE_DEPRECATED;

    /**
     * Return the proxy to the active tool (determining which tool
     * is really, really active is hard when tablets are involved,
     * so leave that to others.
     */
    virtual KoToolProxy * toolProxy() = 0;

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
    virtual KoUnit unit() = 0;

    /**
     * Return the position of the document origin inside the canvas widget.
     * By default the origin of the canvas widget and the position of the
     * document origin are coincident, thus an empty point is returned.
     */
    virtual QPoint documentOrigin() { return QPoint(0,0); };

    /**
     * Return a pointer to the resource provider associated with this
     * canvas. The resource provider contains per-canvas settings such
     * as current foreground and background color.
     */
    KoCanvasResourceProvider * resourceProvider() const;

    /**
     * Return the shape controller for this canvas.
     * A shape controller is used to create or delete shapes and show the relevant dialogs to the user.
     */
    KoShapeController * shapeController() const;

    /*
     * The following 2 methods should be implemented by all canvasses soon so we can work on features
     * like a zoom tool and a way to make the canvasController implement zoom-by-scrollwheel (and ctrl).
     * Specifically; the canvasController should implement these:
     *  void zoomIn(const QPointF &center)
     *  void zoomOut(const QPointF &center)
     *  void zoomTo(const QRectF &newSize)
     *
     * This will allow a new zoom tool in KofficeUI to do everything it needs.
     */
    virtual void setZoom(double zoom) { if(zoom); /* poor mans Q_UNUSED */}
    virtual double zoom() const { return 0;}

private:
    // we need a KoShapeControllerBase so that it can work
    KoCanvasBase();

    class Private;
    Private * const d;
};



#endif // KOCANVASBASE_H
